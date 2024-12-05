#pragma once
#include <string>
#include <xor.hpp>

namespace g
{
    const std::string CS2_PROCESS_NAME{XOR("cs2.exe")};
    const std::string CLIENT_DLL_MODULE_NAME{XOR("client.dll")};
    const std::string ENGINE_DLL_MODULE_NAME{XOR("engine2.dll")};

    namespace toggles
    {
        static bool is_paused{false};
        static bool radar_hack{false};
        static bool glow_hack{false};

        inline void reset_toggles()
        {
            radar_hack = false;
            glow_hack = false;
        }
    }
}
