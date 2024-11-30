#pragma once
#include <string>
#include <xor.hpp>

namespace g
{
    const std::string CS2_PROCESS_NAME{XOR("cs2.exe")};
    const std::string CLIENT_DLL_MODULE_NAME{XOR("client.dll")};

    namespace settings
    {
        static bool is_paused{false};
        static bool radar_hack{false};
    }
}
