#pragma once
#include <xor.hpp>
#include <stdexcept>
#include <vector>
#include <string>
#include <wtypes.h>
#include <cstddef>

class Resource
{
public:
    struct Parameters
    {
        std::size_t size_bytes{0};
        void* ptr{nullptr};
    };

private:
    HRSRC hResource{nullptr};
    HGLOBAL hMemory{nullptr};

    Parameters p;

public:
    Resource(const int& resource_id, const wchar_t* resource_class)
    {
        hResource = FindResource(nullptr, MAKEINTRESOURCE(resource_id), resource_class);
        if (!hResource)
        {
            MessageBox(nullptr, XORW(L"Failed to find resource."), XORW(L"Error"), MB_OK | MB_ICONERROR);
            throw std::runtime_error(XOR("Failed to find resource."));
        }

        hMemory = LoadResource(nullptr, hResource);
        if (!hMemory)
        {
            MessageBox(nullptr, XORW(L"Failed to load resource."), XORW(L"Error"), MB_OK | MB_ICONERROR);
            throw std::runtime_error(XOR("Failed to load resource."));
        }

        p.size_bytes = SizeofResource(nullptr, hResource);
        if (p.size_bytes == 0)
        {
            MessageBox(nullptr, XORW(L"Resource size is zero."), XORW(L"Error"), MB_OK | MB_ICONERROR);
            throw std::runtime_error(XOR("Resource size is zero."));
        }

        p.ptr = LockResource(hMemory);
        if (!p.ptr)
        {
            MessageBox(nullptr, XORW(L"Failed to lock resource"), XORW(L"Error"), MB_OK | MB_ICONERROR);
            throw std::runtime_error(XOR("Failed to lock resource."));
        }
    }

    auto CopyResource() const
    {
        return std::vector<uint8_t>(static_cast<uint8_t*>(p.ptr), static_cast<uint8_t*>(p.ptr) + p.size_bytes);
    }

    std::size_t GetDataSize() const
    {
        return p.size_bytes;
    }

    void* GetData() const
    {
        return p.ptr;
    }
};
