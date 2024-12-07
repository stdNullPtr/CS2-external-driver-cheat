#pragma once
#include <Windows.h>
#include <cstdint>
#include <iostream>
#include <xor.hpp>
#include <winioctl.h>

namespace driver
{
    // TODO this duplication between user mode and kernel mode is error prone
    struct Request
    {
        HANDLE process_id_handle;

        PVOID target_address;
        PVOID buffer;

        SIZE_T size;
        SIZE_T return_size;
    };

    // TODO this duplication between user mode and kernel mode is error prone
    namespace control_codes
    {
        constexpr ULONG attach{CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA1, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)};
        constexpr ULONG read{CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)};
        constexpr ULONG write{CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA3, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)};
    }

    class Driver
    {
    private:
        const HANDLE handle_;

    private:
        static HANDLE create_handle();

    public:
        Driver();
        ~Driver();
        Driver(const Driver& other) = delete;
        Driver(Driver&& other) noexcept = delete;
        Driver& operator=(const Driver& other) = delete;
        Driver& operator=(Driver&& other) noexcept = delete;

        [[nodiscard]] bool attach(const DWORD& process_id) const;
        [[nodiscard]] bool is_valid() const;

        //TODO optional
        template <class T>
        T read(const std::uintptr_t& address) const
        {
            T result_buffer{};

            Request driver_request;
            driver_request.target_address = reinterpret_cast<PVOID>(address);
            driver_request.buffer = &result_buffer;
            driver_request.size = sizeof(T);

            DeviceIoControl(handle_, control_codes::read, &driver_request, sizeof(driver_request), &driver_request, sizeof(driver_request), nullptr, nullptr);

            return result_buffer;
        }

        template <class T>
        void write(const std::uintptr_t& address, const T& value) const
        {
            Request driver_request;
            driver_request.target_address = reinterpret_cast<PVOID>(address);
            driver_request.buffer = (PVOID)&value;
            driver_request.size = sizeof(T);

            DeviceIoControl(handle_, control_codes::write, &driver_request, sizeof(driver_request), &driver_request, sizeof(driver_request), nullptr, nullptr);
        }
    };
}
