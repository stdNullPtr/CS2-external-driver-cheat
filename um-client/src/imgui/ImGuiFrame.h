#pragma once

#include "lib/imgui.h"
#include "lib/backends/imgui_impl_win32.h"
#include "lib/backends/imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cheat::imgui
{
    struct FrameContext
    {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64 FenceValue;
    };

    struct ExampleDescriptorHeapAllocator
    {
        ID3D12DescriptorHeap* Heap = nullptr;
        D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
        D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
        D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
        UINT HeapHandleIncrement;
        ImVector<int> FreeIndices;

        void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
        {
            IM_ASSERT(Heap == nullptr && FreeIndices.empty());
            Heap = heap;
            D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
            HeapType = desc.Type;
            HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
            HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
            HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
            FreeIndices.reserve((int)desc.NumDescriptors);
            for (int n = desc.NumDescriptors; n > 0; n--)
                FreeIndices.push_back(n);
        }

        void Destroy()
        {
            Heap = NULL;
            FreeIndices.clear();
        }

        void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
        {
            IM_ASSERT(FreeIndices.Size > 0);
            int idx = FreeIndices.back();
            FreeIndices.pop_back();
            out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
            out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
        }

        void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
        {
            int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
            int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
            IM_ASSERT(cpu_idx == gpu_idx);
            FreeIndices.push_back(cpu_idx);
        }
    };

    namespace g
    {
        static const int APP_NUM_FRAMES_IN_FLIGHT = 3; //TODO this is cheat toggles
        static const int APP_NUM_BACK_BUFFERS = 3; //TODO this is cheat toggles
        static const int APP_SRV_HEAP_SIZE = 64; //TODO this is cheat toggles

        static FrameContext g_frameContext[APP_NUM_FRAMES_IN_FLIGHT] = {};
        static UINT g_frameIndex = 0;

        static ID3D12Device* g_pd3dDevice = nullptr;
        static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
        static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
        static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;
        static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
        static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
        static ID3D12Fence* g_fence = nullptr;
        static HANDLE g_fenceEvent = nullptr;
        static UINT64 g_fenceLastSignaledValue = 0;
        static IDXGISwapChain3* g_pSwapChain = nullptr;
        static bool g_SwapChainOccluded = false;
        static HANDLE g_hSwapChainWaitableObject = nullptr;
        static ID3D12Resource* g_mainRenderTargetResource[APP_NUM_BACK_BUFFERS] = {};
        static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[APP_NUM_BACK_BUFFERS] = {};
    }

    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    void WaitForLastSubmittedFrame();
    FrameContext* WaitForNextFrameResources();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void init()
    {
        WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr};
        RegisterClassExW(&wc);
        HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX12 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

        // Initialize Direct3D
        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return;
        }

        // Show the window
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(hwnd);

        ImGui_ImplDX12_InitInfo init_info = {};
        init_info.Device = g::g_pd3dDevice;
        init_info.CommandQueue = g::g_pd3dCommandQueue;
        init_info.NumFramesInFlight = g::APP_NUM_FRAMES_IN_FLIGHT;
        init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        init_info.SrvDescriptorHeap = g::g_pd3dSrvDescHeap;
        init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g::g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
        init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g::g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
        ImGui_ImplDX12_Init(&init_info);
    }

    namespace frame
    {
        enum FRAME_STATE
        {
            FRAME_QUIT,
            FRAME_SUCCESS,
            FRAME_SKIP
        };

        void cleanup()
        {
            WaitForLastSubmittedFrame();

            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupDeviceD3D();
        }

        FRAME_STATE startFrame()
        {
            // Poll and handle messages (inputs, window resize, etc.)
            // See the WndProc() function below for our to dispatch events to the Win32 backend.
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    return FRAME_QUIT;
            }

            // Handle window screen locked
            if (g::g_SwapChainOccluded && g::g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
            {
                Sleep(10);
                return FRAME_SKIP;
            }
            g::g_SwapChainOccluded = false;

            // Start the Dear ImGui frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            return FRAME_SUCCESS;
        }

        void render(const ImVec4 clear_color)
        {
            // Rendering
            ImGui::Render();

            FrameContext* frameCtx = WaitForNextFrameResources();
            UINT backBufferIdx = g::g_pSwapChain->GetCurrentBackBufferIndex();
            frameCtx->CommandAllocator->Reset();

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = g::g_mainRenderTargetResource[backBufferIdx];
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            g::g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
            g::g_pd3dCommandList->ResourceBarrier(1, &barrier);

            // Render Dear ImGui graphics
            const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
            g::g_pd3dCommandList->ClearRenderTargetView(g::g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
            g::g_pd3dCommandList->OMSetRenderTargets(1, &g::g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
            g::g_pd3dCommandList->SetDescriptorHeaps(1, &g::g_pd3dSrvDescHeap);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g::g_pd3dCommandList);
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            g::g_pd3dCommandList->ResourceBarrier(1, &barrier);
            g::g_pd3dCommandList->Close();

            g::g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g::g_pd3dCommandList);

            // Present
            HRESULT hr = g::g_pSwapChain->Present(1, 0); // Present with vsync
            //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
            g::g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

            UINT64 fenceValue = g::g_fenceLastSignaledValue + 1;
            g::g_pd3dCommandQueue->Signal(g::g_fence, fenceValue);
            g::g_fenceLastSignaledValue = fenceValue;
            frameCtx->FenceValue = fenceValue;
        }
    }

#pragma region imgui_helpers
    bool CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC1 sd;
        {
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = g::APP_NUM_BACK_BUFFERS;
            sd.Width = 0;
            sd.Height = 0;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            sd.Scaling = DXGI_SCALING_STRETCH;
            sd.Stereo = FALSE;
        }

        // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
        ID3D12Debug* pdx12Debug = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
            pdx12Debug->EnableDebugLayer();
#endif

        // Create device
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g::g_pd3dDevice)) != S_OK)
            return false;

        // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
        if (pdx12Debug != nullptr)
        {
            ID3D12InfoQueue* pInfoQueue = nullptr;
            g::g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            pInfoQueue->Release();
            pdx12Debug->Release();
        }
#endif

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = g::APP_NUM_BACK_BUFFERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (g::g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g::g_pd3dRtvDescHeap)) != S_OK)
                return false;

            SIZE_T rtvDescriptorSize = g::g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g::g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < g::APP_NUM_BACK_BUFFERS; i++)
            {
                g::g_mainRenderTargetDescriptor[i] = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = g::APP_SRV_HEAP_SIZE;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (g::g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g::g_pd3dSrvDescHeap)) != S_OK)
                return false;
            g::g_pd3dSrvDescHeapAlloc.Create(g::g_pd3dDevice, g::g_pd3dSrvDescHeap);
        }

        {
            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;
            if (g::g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g::g_pd3dCommandQueue)) != S_OK)
                return false;
        }

        for (UINT i = 0; i < g::APP_NUM_FRAMES_IN_FLIGHT; i++)
            if (g::g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g::g_frameContext[i].CommandAllocator)) != S_OK)
                return false;

        if (g::g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g::g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g::g_pd3dCommandList)) != S_OK ||
            g::g_pd3dCommandList->Close() != S_OK)
            return false;

        if (g::g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g::g_fence)) != S_OK)
            return false;

        g::g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (g::g_fenceEvent == nullptr)
            return false;

        {
            IDXGIFactory4* dxgiFactory = nullptr;
            IDXGISwapChain1* swapChain1 = nullptr;
            if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
                return false;
            if (dxgiFactory->CreateSwapChainForHwnd(g::g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
                return false;
            if (swapChain1->QueryInterface(IID_PPV_ARGS(&g::g_pSwapChain)) != S_OK)
                return false;
            swapChain1->Release();
            dxgiFactory->Release();
            g::g_pSwapChain->SetMaximumFrameLatency(g::APP_NUM_BACK_BUFFERS);
            g::g_hSwapChainWaitableObject = g::g_pSwapChain->GetFrameLatencyWaitableObject();
        }

        CreateRenderTarget();
        return true;
    }

    void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g::g_pSwapChain)
        {
            g::g_pSwapChain->SetFullscreenState(false, nullptr);
            g::g_pSwapChain->Release();
            g::g_pSwapChain = nullptr;
        }
        if (g::g_hSwapChainWaitableObject != nullptr) { CloseHandle(g::g_hSwapChainWaitableObject); }
        for (UINT i = 0; i < g::APP_NUM_FRAMES_IN_FLIGHT; i++)
            if (g::g_frameContext[i].CommandAllocator)
            {
                g::g_frameContext[i].CommandAllocator->Release();
                g::g_frameContext[i].CommandAllocator = nullptr;
            }
        if (g::g_pd3dCommandQueue)
        {
            g::g_pd3dCommandQueue->Release();
            g::g_pd3dCommandQueue = nullptr;
        }
        if (g::g_pd3dCommandList)
        {
            g::g_pd3dCommandList->Release();
            g::g_pd3dCommandList = nullptr;
        }
        if (g::g_pd3dRtvDescHeap)
        {
            g::g_pd3dRtvDescHeap->Release();
            g::g_pd3dRtvDescHeap = nullptr;
        }
        if (g::g_pd3dSrvDescHeap)
        {
            g::g_pd3dSrvDescHeap->Release();
            g::g_pd3dSrvDescHeap = nullptr;
        }
        if (g::g_fence)
        {
            g::g_fence->Release();
            g::g_fence = nullptr;
        }
        if (g::g_fenceEvent)
        {
            CloseHandle(g::g_fenceEvent);
            g::g_fenceEvent = nullptr;
        }
        if (g::g_pd3dDevice)
        {
            g::g_pd3dDevice->Release();
            g::g_pd3dDevice = nullptr;
        }

#ifdef DX12_ENABLE_DEBUG_LAYER
        IDXGIDebug1* pDebug = nullptr;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
            pDebug->Release();
        }
#endif
    }

    void CreateRenderTarget()
    {
        for (UINT i = 0; i < g::APP_NUM_BACK_BUFFERS; i++)
        {
            ID3D12Resource* pBackBuffer = nullptr;
            g::g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            g::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g::g_mainRenderTargetDescriptor[i]);
            g::g_mainRenderTargetResource[i] = pBackBuffer;
        }
    }

    void CleanupRenderTarget()
    {
        WaitForLastSubmittedFrame();

        for (UINT i = 0; i < g::APP_NUM_BACK_BUFFERS; i++)
            if (g::g_mainRenderTargetResource[i])
            {
                g::g_mainRenderTargetResource[i]->Release();
                g::g_mainRenderTargetResource[i] = nullptr;
            }
    }

    void WaitForLastSubmittedFrame()
    {
        FrameContext* frameCtx = &g::g_frameContext[g::g_frameIndex % g::APP_NUM_FRAMES_IN_FLIGHT];

        UINT64 fenceValue = frameCtx->FenceValue;
        if (fenceValue == 0)
            return; // No fence was signaled

        frameCtx->FenceValue = 0;
        if (g::g_fence->GetCompletedValue() >= fenceValue)
            return;

        g::g_fence->SetEventOnCompletion(fenceValue, g::g_fenceEvent);
        WaitForSingleObject(g::g_fenceEvent, INFINITE);
    }

    FrameContext* WaitForNextFrameResources()
    {
        UINT nextFrameIndex = g::g_frameIndex + 1;
        g::g_frameIndex = nextFrameIndex;

        HANDLE waitableObjects[] = {g::g_hSwapChainWaitableObject, nullptr};
        DWORD numWaitableObjects = 1;

        FrameContext* frameCtx = &g::g_frameContext[nextFrameIndex % g::APP_NUM_FRAMES_IN_FLIGHT];
        UINT64 fenceValue = frameCtx->FenceValue;
        if (fenceValue != 0) // means no fence was signaled
        {
            frameCtx->FenceValue = 0;
            g::g_fence->SetEventOnCompletion(fenceValue, g::g_fenceEvent);
            waitableObjects[1] = g::g_fenceEvent;
            numWaitableObjects = 2;
        }

        WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

        return frameCtx;
    }

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (g::g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                WaitForLastSubmittedFrame();
                CleanupRenderTarget();
                HRESULT result = g::g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
                assert(SUCCEEDED(result) && "Failed to resize swapchain.");
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

#pragma endregion imgui_helpers
}
