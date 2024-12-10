#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <chrono>
#include <Windows.h>
#include <thread>

#include "driver/Driver.hpp"
#include "process.hpp"
#include "console.hpp"
#include "window.hpp"
#include "controller/Cs2CheatController.hpp"
#include "controller/Entity.hpp"
#include "imgui/ImGuiFrame.h"
#include <io.h>
#include <fcntl.h>

using namespace std::chrono_literals;
using namespace g::toggles;
using namespace commons::console;
using std::this_thread::sleep_for;

int main()
{
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);

    (void)std::setlocale(LC_ALL, "en_US.utf8");

    setCursorVisibility(false);

    cheat::imgui::init();

    const driver::Driver driver{};
    if (!driver.is_valid())
    {
        MessageBox(nullptr, XORW(L"Load the Driver first lmao."), XORW(L"Oopsie"), MB_OK);
        return EXIT_FAILURE;
    }

    cheat::Cs2CheatController cheat;

    sleep_for(2s);

    while (!(GetAsyncKeyState(VK_END) & 0x1))
    {
        const auto frameState{cheat::imgui::frame::startFrame()};
        if (frameState == cheat::imgui::frame::FRAME_QUIT)
        {
            break;
        }

        if (frameState == cheat::imgui::frame::FRAME_SKIP)
        {
            continue;
        }

        if (show_menu)
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 230));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

            ImGui::Begin(XOR("XD"), &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 128, 255, 255));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
            ImGui::SeparatorText(XOR("Toggles"));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(50, 50, 50, 255));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(70, 70, 70, 255));
            ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(0, 128, 255, 255));
            ImGui::Checkbox(XOR("[F1] Pause"), &is_paused);
            if (is_paused)
            {
                ImGui::BeginDisabled();
            }
            ImGui::Checkbox(XOR("[F2] Radar (Could get flagged, use with caution)"), &radar_hack);
            ImGui::Checkbox(XOR("[F3] Glow (Could get flagged, use with caution)"), &glow_hack);
            if (is_paused)
            {
                ImGui::EndDisabled();
            }
            ImGui::PopStyleColor(3);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 0, 0, 255));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
            ImGui::SeparatorText(XOR("[INS] Close"));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::End();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();
        }

        cheat::imgui::frame::render();

        clearConsole({0, 0});

        if (!commons::window::waitForWindow(XORW(L"Counter-Strike 2")))
        {
            std::wcerr << XORW(L"Aborted looking for game window, exiting...\n");
            break;
        }

        if (!cheat.is_state_valid())
        {
            std::wcout << XORW(L"Controller state invalid, will re-init...\n");
            reset_toggles();
            sleep_for(3s);

            if (!cheat.init(driver))
            {
                std::wcerr << XORW(L"Failed initializing cheat controller state, retying...\n");
                sleep_for(2s);
                continue;
            }
        }

        std::wcout << ansi_codes::red << XORW(L"[END] Quit\n") << ansi_codes::reset_color;
        if (!cheat.is_in_game(driver))
        {
            std::wcout << XORW(L"Waiting for you to join game...\n");
            sleep_for(500ms);
            continue;
        }

        std::wcout << '\n';
        std::wcout << ansi_codes::light_blue << XORW(L"[F1] Pause\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[F2] Radar hack\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[F3] Glow hack\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[INS] Glow hack\n") << ansi_codes::reset_color;
        std::wcout << '\n';

        if (GetAsyncKeyState(VK_F1) & 0x1)
        {
            is_paused = !is_paused;
        }

        if (is_paused)
        {
            std::wcout << XORW(L"[F1] Paused...\n");
            sleep_for(50ms);
            continue;
        }

        if (GetAsyncKeyState(VK_F2) & 0x1)
        {
            radar_hack = !radar_hack;
        }

        if (GetAsyncKeyState(VK_F3) & 0x1)
        {
            glow_hack = !glow_hack;
        }

        if (GetAsyncKeyState(VK_INSERT) & 0x1)
        {
            show_menu = !show_menu;
        }

        const auto me{cheat.get_local_player(driver)};

        const auto my_eye_pos{me.get_eye_pos(driver)};
        const auto my_forward_vec{me.get_forward_vector(driver)};
        const auto my_team{me.get_team(driver)};

        std::wcout << XORW(L"Forward vec: ") << my_forward_vec << '\n'
            << XORW(L"My eye position: ") << my_eye_pos << '\n';

        std::wcout << '\n';

        for (int i{1}; i < 32; i++)
        {
            const std::optional entity{cheat.get_entity_from_list(driver, i)};

            if (!entity.has_value())
            {
                continue;
            }

            const auto is_local_player{entity->is_local_player(driver)};
            const auto player_team{entity->get_team(driver)};
            const auto player_health{entity->get_health(driver)};
            const auto entity_spotted{entity->is_spotted(driver)};
            const auto is_glowing{entity->is_glowing(driver)};

            std::wcout << XORW(L"Ent: ") << i << '\n'
                << XORW(L"Name: ") << entity->get_name(driver) << '\n'
                << XORW(L"Is this me: ") << (is_local_player ? "yes" : "no") << '\n'
                << XORW(L"Enemy: ") << (my_team != player_team ? "yes" : "no") << '\n'
                << XORW(L"HP: ") << std::dec << player_health << '\n'
                << XORW(L"Visible on Radar: ") << (entity_spotted ? "yes" : "no") << '\n'
                << XORW(L"Is glowing: ") << (is_glowing ? "yes" : "no") << '\n';

            if (my_team != player_team && player_health > 0 && !is_local_player)
            {
                if (glow_hack)
                {
                    entity->set_glowing(driver, true);
                }

                if (radar_hack && !entity_spotted)
                {
                    //TODO should we set the correct mask as well? we are setting this bool but the variables after it should be 1 as well
                    entity->set_spotted(driver, true);
                }
            }
            std::wcout << '\n';
        }

        sleep_for(20ms);
    }
    cheat::imgui::frame::cleanup();

    return EXIT_SUCCESS;
}
