#pragma once

#include <d3d11.h>

#include "lib/imgui.h"
#include "lib/backends/imgui_impl_win32.h"
#include "lib/backends/imgui_impl_dx11.h"
#include "lib/misc/freetype/imgui_freetype.h"
#include "lib/misc/font/IconsFontAwesome5.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cheat::imgui
{
    namespace g
    {
        static inline ID3D11Device* g_pd3dDevice{ nullptr };
        static inline ID3D11DeviceContext* g_pd3dDeviceContext{ nullptr };
        static inline IDXGISwapChain* g_pSwapChain{ nullptr };
        static inline bool g_SwapChainOccluded{ false };
        static inline UINT g_ResizeWidth{ 0 };
        static inline UINT g_ResizeHeight{ 0 };
        static inline ID3D11RenderTargetView* g_mainRenderTargetView{ nullptr };

        static inline WNDCLASSEXW wc;
        static inline HWND hOverlay;
        constexpr auto overlay_window_name{ L"zzxzz" };
        constexpr auto font_size{ 20.0f };
    }

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    inline void init()
    {
        g::wc = WNDCLASSEXW{
            sizeof(g::wc),
            CS_HREDRAW | CS_VREDRAW,
            WndProc,
            0L,
            0L,
            GetModuleHandle(nullptr),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            L"win",
            nullptr
        };

        RegisterClassExW(&g::wc);

        g::hOverlay = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
            g::wc.lpszClassName,
            g::overlay_window_name,
            WS_POPUP,
            0,
            0,
            ::g::screen_width,
            ::g::screen_height,
            nullptr,
            nullptr,
            g::wc.hInstance,
            nullptr);

        SetLayeredWindowAttributes(g::hOverlay, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

        // Initialize Direct3D
        if (!CreateDeviceD3D(g::hOverlay))
        {
            CleanupDeviceD3D();
            UnregisterClassW(g::wc.lpszClassName, g::wc.hInstance);
            return;
        }

        // Show the window
        ShowWindow(g::hOverlay, SW_SHOWDEFAULT);
        UpdateWindow(g::hOverlay);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io{ ImGui::GetIO() };
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(g::hOverlay);
        ImGui_ImplDX11_Init(g::g_pd3dDevice, g::g_pd3dDeviceContext);

        float baseFontSize = g::font_size;
        float iconFontSize = baseFontSize * 2.0f / 3.0f;

        static constexpr ImWchar ranges[]{ 0x1, 0x1FFFF, 0 };
        static ImFontConfig cfg;
        // cfg.OversampleH = cfg.OversampleV = 5;
        cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
        //cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
        //cfg.RasterizerMultiply = 1.5f;
        io.Fonts->AddFontFromFileTTF(XOR("NotoSans-Regular.ttf"), baseFontSize, &cfg, ranges);

        static constexpr ImWchar icons_ranges[]{ ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        static ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = iconFontSize;
        // icons_config.OversampleH = icons_config.OversampleV = 3;
        io.Fonts->AddFontFromFileTTF(XOR(FONT_ICON_FILE_NAME_FAS), iconFontSize, &icons_config, icons_ranges);
    }

    namespace frame
    {
        enum FRAME_STATE
        {
            FRAME_QUIT,
            FRAME_SUCCESS,
            FRAME_SKIP
        };

        inline void cleanup()
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupDeviceD3D();
            DestroyWindow(g::hOverlay);
            UnregisterClassW(g::wc.lpszClassName, g::wc.hInstance);
        }

        inline FRAME_STATE startFrame()
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
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            return FRAME_SUCCESS;
        }

        inline void render()
        {
            ImGui::Render();
            constexpr float transparent[4]{ 0, 0, 0, 0 };
            g::g_pd3dDeviceContext->OMSetRenderTargets(1, &g::g_mainRenderTargetView, nullptr);
            g::g_pd3dDeviceContext->ClearRenderTargetView(g::g_mainRenderTargetView, transparent);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // Present
            HRESULT hr{ g::g_pSwapChain->Present(1, 0) }; // Present with vsync
            //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
            g::g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
        }
    }

#pragma region imgui_helpers
    inline bool CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g::g_pSwapChain, &g::g_pd3dDevice, &featureLevel, &g::g_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g::g_pSwapChain, &g::g_pd3dDevice, &featureLevel, &g::g_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    inline void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g::g_pSwapChain)
        {
            g::g_pSwapChain->Release();
            g::g_pSwapChain = nullptr;
        }
        if (g::g_pd3dDeviceContext)
        {
            g::g_pd3dDeviceContext->Release();
            g::g_pd3dDeviceContext = nullptr;
        }
        if (g::g_pd3dDevice)
        {
            g::g_pd3dDevice->Release();
            g::g_pd3dDevice = nullptr;
        }
    }

    inline void CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g::g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g::g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    inline void CleanupRenderTarget()
    {
        if (g::g_mainRenderTargetView)
        {
            g::g_mainRenderTargetView->Release();
            g::g_mainRenderTargetView = nullptr;
        }
    }

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            g::g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
            g::g_ResizeHeight = (UINT)HIWORD(lParam);
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
