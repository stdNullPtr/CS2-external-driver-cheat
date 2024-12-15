#pragma once
#include <string>
#include <XOR.hpp>

#include "imgui/lib/imgui.h"

namespace g
{
    static const std::wstring cs2_process_name{XORW(L"cs2.exe")};
    static const std::wstring cs2_window_name{XORW(L"Counter-Strike 2")};
    static const std::wstring client_dll_module_name{XORW(L"client.dll")};
    static const std::wstring engine_dll_module_name{XORW(L"engine2.dll")};
    static const int screen_width{GetSystemMetrics(SM_CXSCREEN)};
    static const int screen_height{GetSystemMetrics(SM_CYSCREEN)};
    static const ImVec2 screen_center{screen_width / 2.0f, screen_height / 2.0f};

    static ImVec4 esp_color{255.0f, 0.0f, 0.0f, 255.0f};
    static float esp_box_thickness{1.0f};
    static ImVec4 esp_health_color{0.0f, 255.0f, 0.0f, 255.0f};
    static ImVec4 text_color{0.0f, 255.0f, 0.0f, 255.0f};
    static ImVec4 weapon_awp_text_color{255.0f, 0.0f, 0.0f, 255.0f};
    static ImVec4 weapon_knife_text_color{0.0f, 0.0f, 255.0f, 255.0f};
    static ImVec4 additional_screen_info_text_color{255.0f, 0.0f, 0.0f, 255.0f};
    static int additional_screen_info_position_x{100};
    static int additional_screen_info_position_y{350};

    static float aim_fov{50.0f};

    namespace toggles
    {
        //TODO atomic
        static bool is_paused{false};
        static bool show_menu{false};
        static bool radar_hack{false};
        static bool glow_hack{false};
        static bool no_flash_hack{false};
        static bool esp_hack{false};
        static bool aim_hack{false};

        inline void reset_toggles()
        {
            show_menu = false;

            radar_hack = false;
            glow_hack = false;
            no_flash_hack = false;
            esp_hack = false;
            aim_hack = false;
        }
    }
}
