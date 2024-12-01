#include "driver.hpp"

namespace driver
{
    driver::driver(): m_handle(create_handle())
    {
    }

    driver::~driver()
    {
        CloseHandle(m_handle);
        std::cout << XOR("Driver handle successfully destroyed.\n");
    }

    bool driver::is_valid() const
    {
        return m_handle != INVALID_HANDLE_VALUE;
    }

    HANDLE driver::create_handle()
    {
        const HANDLE driver_handle{
            CreateFile(
                R"(\\.\xd)",
                GENERIC_READ,
                0,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr)
        };

        if (driver_handle == INVALID_HANDLE_VALUE)
        {
            std::cerr << XOR("Failed creating driver handle. Error: ") << GetLastError() << '\n';
        }
        else
        {
            std::cout << XOR("Created driver handle.\n");
        }

        return driver_handle;
    }

    bool driver::attach(const DWORD& process_id) const
    {
        Request driverRequest;
        driverRequest.processIdHandle = reinterpret_cast<HANDLE>(process_id);

        return DeviceIoControl(m_handle, control_codes::attach, &driverRequest, sizeof(driverRequest), &driverRequest, sizeof(driverRequest), nullptr, nullptr);
    }
}
