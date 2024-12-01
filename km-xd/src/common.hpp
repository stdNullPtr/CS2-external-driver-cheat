#pragma once
#include <ntifs.h>

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
        constexpr ULONG attach{ CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA1, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) };
        constexpr ULONG read{ CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA2, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) };
        constexpr ULONG write{ CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA3, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) };
    }
}