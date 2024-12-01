#include "driver.hpp"

namespace driver
{
    driver::driver(): handle(create_handle())
    {
    }

    driver::~driver()
    {
        std::cout << XOR("Driver handle destroyed.\n");
        CloseHandle(handle);
    }

    HANDLE driver::create_handle()
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

    bool driver::attach(const DWORD& process_id) const
    {
        Request driverRequest;
        driverRequest.processIdHandle = reinterpret_cast<HANDLE>(process_id);

        return DeviceIoControl(handle, control_codes::attach, &driverRequest, sizeof(driverRequest), &driverRequest, sizeof(driverRequest), nullptr, nullptr);
    }
}
