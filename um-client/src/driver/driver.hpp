#pragma once
#include <Windows.h>
#include <cstdint>

#include "../../km-xd/src/common.hpp"

namespace driver
{
    class Driver
    {
    public:
        const HANDLE handle;

        Driver();

        bool attach(const DWORD& processId) const;

        static HANDLE createHandle();

        template <class T>
        T read(const std::uintptr_t& address) const
        {
            T resultBuffer{};

            Request driverRequest;
            driverRequest.targetAddress = reinterpret_cast<PVOID>(address);
            driverRequest.buffer = &resultBuffer;
            driverRequest.size = sizeof(T);

            DeviceIoControl(handle, control_codes::read, &driverRequest, sizeof(driverRequest), &driverRequest,
                            sizeof(driverRequest), nullptr, nullptr);

            return resultBuffer;
        }

        template <class T>
        void write(const std::uintptr_t& address, const T& value) const
        {
            Request driverRequest;
            driverRequest.targetAddress = reinterpret_cast<PVOID>(address);
            driverRequest.buffer = (PVOID)&value;
            driverRequest.size = sizeof(T);

            DeviceIoControl(handle, control_codes::write, &driverRequest, sizeof(driverRequest), &driverRequest,
                            sizeof(driverRequest), nullptr, nullptr);
        }
    };
}
