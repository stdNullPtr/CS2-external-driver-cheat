#pragma once
#include <string>
#include <XOR.hpp>

#include "imgui/lib/imgui.h"

namespace g
{
    static const std::wstring CS2_PROCESS_NAME{XORW(L"cs2.exe")};
    static const std::wstring CS2_WINDOW_NAME{XORW(L"Counter-Strike 2")};
    static const std::wstring CLIENT_DLL_MODULE_NAME{XORW(L"client.dll")};
    static const std::wstring ENGINE_DLL_MODULE_NAME{XORW(L"engine2.dll")};
    static const int screen_width{GetSystemMetrics(SM_CXSCREEN)};
    static const int screen_height{GetSystemMetrics(SM_CYSCREEN)};

    static ImVec4 espColor{255.0f, 0.0f, 0.0f, 255.0f};
    static float espBoxThickness{1.0f};
    static ImVec4 espHealthColor{0.0f, 255.0f, 0.0f, 255.0f};
    static ImVec4 textColor{0.0f, 255.0f, 0.0f, 255.0f};
    static ImVec4 weaponAwpTextColor{255.0f, 0.0f, 0.0f, 255.0f};
    static ImVec4 weaponKnifeTextColor{0.0f,0.0f, 255.0f, 255.0f};

    namespace toggles
    {
        //TODO atomic
        static bool is_paused{false};
        static bool show_menu{false};
        static bool radar_hack{false};
        static bool glow_hack{false};
        static bool esp_hack{false};

        inline void reset_toggles()
        {
            show_menu = false;

            radar_hack = false;
            glow_hack = false;
            esp_hack = false;
        }
    }
}
