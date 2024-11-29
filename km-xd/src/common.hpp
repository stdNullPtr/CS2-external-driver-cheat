#pragma once

namespace driver
{
    struct Request
    {
        HANDLE processIdHandle;

        PVOID targetAddress;
        PVOID buffer;

        SIZE_T size;
        SIZE_T returnSize;
    };

    namespace control_codes
    {
        constexpr ULONG attach{ CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA1, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) };
        constexpr ULONG read{ CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) };
        constexpr ULONG write{ CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA3, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) };
    }
}