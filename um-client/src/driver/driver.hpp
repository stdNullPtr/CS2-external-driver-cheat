#pragma once
#include <Windows.h>
#include <cstdint>
#include <iostream>
#include <xor.hpp>
#include <winioctl.h>
#include <optional>

namespace driver
{
    // TODO this duplication between user mode and kernel mode is error prone
    struct Request
    {
        HANDLE processIdHandle;

        PVOID targetAddress;
        PVOID buffer;

        SIZE_T size;
        SIZE_T returnSize;
    };

    // TODO this duplication between user mode and kernel mode is error prone
    namespace control_codes
    {
        constexpr ULONG attach{CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA1, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)};
        constexpr ULONG read{CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)};
        constexpr ULONG write{CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA3, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)};
    }

    class driver
    {
    private:
        const HANDLE handle;

    private:
        static HANDLE create_handle();

    public:
        driver();
        ~driver();
        driver(const driver& other) = delete;
        driver(driver&& other) noexcept = delete;
        driver& operator=(const driver& other) = delete;
        driver& operator=(driver&& other) noexcept = delete;

        bool attach(const DWORD& process_id) const;

        //TODO optional
        template <class T>
        T read(const std::uintptr_t& address) const
        {
            T resultBuffer{};

            Request driverRequest;
            driverRequest.targetAddress = reinterpret_cast<PVOID>(address);
            driverRequest.buffer = &resultBuffer;
            driverRequest.size = sizeof(T);

            DeviceIoControl(handle, control_codes::read, &driverRequest, sizeof(driverRequest), &driverRequest, sizeof(driverRequest), nullptr, nullptr);

            return resultBuffer;
        }

        template <class T>
        void write(const std::uintptr_t& address, const T& value) const
        {
            Request driverRequest;
            driverRequest.targetAddress = reinterpret_cast<PVOID>(address);
            driverRequest.buffer = (PVOID)&value;
            driverRequest.size = sizeof(T);

            DeviceIoControl(handle, control_codes::write, &driverRequest, sizeof(driverRequest), &driverRequest, sizeof(driverRequest), nullptr, nullptr);
        }
    };
}
