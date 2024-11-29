#include "driver.hpp"

namespace driver
{
    Driver::Driver(): handle(createHandle())
    {
    }

    Driver::~Driver()
    {
        CloseHandle(handle);
    }

    HANDLE Driver::createHandle()
    {
        const HANDLE driverHandle{
            CreateFile(
                R"(\\.\xd)",
                GENERIC_READ,
                0,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr)
        };

        if (driverHandle == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error(XOR("Failed creating driver handle. Error: ") + std::to_string(GetLastError()));
        }
        std::cout << XOR("Created driver handle.\n");

        return driverHandle;
    }

    bool Driver::attach(const DWORD& processId) const
    {
        Request driverRequest;
        driverRequest.processIdHandle = reinterpret_cast<HANDLE>(processId);

        return DeviceIoControl(handle, control_codes::attach, &driverRequest, sizeof(driverRequest), &driverRequest,
                               sizeof(driverRequest), nullptr, nullptr);
    }
}
