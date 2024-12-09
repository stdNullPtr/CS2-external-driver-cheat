#include "Driver.hpp"

namespace driver
{
    Driver::Driver(): handle_(create_handle())
    {
    }

    Driver::~Driver()
    {
        CloseHandle(handle_);
        std::wcout << XORW(L"Driver handle successfully destroyed.\n");
    }

    bool Driver::is_valid() const
    {
        return handle_ != INVALID_HANDLE_VALUE;
    }

    HANDLE Driver::create_handle()
    {
        const HANDLE driver_handle{
            CreateFile(
                XORW(LR"(\\.\xd)"),
                GENERIC_READ,
                0,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr)
        };

        if (driver_handle == INVALID_HANDLE_VALUE)
        {
            std::wcerr << XORW(L"Failed creating Driver handle. Error: ") << GetLastError() << '\n';
        }
        else
        {
            std::wcout << XORW(L"Created Driver handle.\n");
        }

        return driver_handle;
    }

    bool Driver::attach(const DWORD& process_id) const
    {
        Request driver_request;
        driver_request.process_id_handle = reinterpret_cast<HANDLE>(process_id);

        return DeviceIoControl(handle_, control_codes::attach, &driver_request, sizeof(driver_request), &driver_request, sizeof(driver_request), nullptr, nullptr);
    }
}
