#pragma once
#include <string>
#include <XOR.hpp>

namespace g
{
    const std::wstring CS2_PROCESS_NAME{XORW(L"cs2.exe")};
    const std::wstring CLIENT_DLL_MODULE_NAME{XORW(L"client.dll")};
    const std::wstring ENGINE_DLL_MODULE_NAME{XORW(L"engine2.dll")};

    namespace toggles
    {
        static bool is_paused{false};
        static bool show_menu{false};
        static bool radar_hack{false};
        static bool glow_hack{false};

        inline void reset_toggles()
        {
            show_menu = false;

            radar_hack = false;
            glow_hack = false;
        }
    }
}
