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

#include "draw/Draw.hpp"

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

    const driver::Driver driver{};
    if (!driver.is_valid())
    {
        MessageBox(nullptr, XORW(L"Load the Driver first lmao."), XORW(L"Oopsie"), MB_OK);
        return EXIT_FAILURE;
    }

    cheat::Cs2CheatController cheat;

    draw::EspDrawList draw_list;
    std::jthread draw_thread(draw::draw_imgui, std::ref(draw_list));

    sleep_for(2s);

    while (!(GetAsyncKeyState(VK_END) & 0x1))
    {
#ifndef NDEBUG
        clearConsole({0, 0});
#endif
        if (!cheat.is_state_valid())
        {
            std::wcout << XORW(L"Controller state invalid, will re-init...\n");

            // prevent any leftover draw artifacts
            draw_list.clear();
            reset_toggles();

            if (!commons::window::waitForWindow(XORW(L"Counter-Strike 2")))
            {
                std::wcout << XORW(L"Aborted looking for game window, exiting...\n");
                return EXIT_SUCCESS;
            }

            sleep_for(3s);

            if (!cheat.init(driver))
            {
                std::wcerr << XORW(L"Failed initializing cheat controller state, retying...\n");
                sleep_for(2s);
                continue;
            }
        }

#ifndef NDEBUG
        std::wcout << ansi_codes::red << XORW(L"[END] Quit\n") << ansi_codes::reset_color;
#endif

        if (!cheat.is_in_game(driver))
        {
            std::wcout << XORW(L"Waiting for you to join game...\n");
            sleep_for(500ms);
            continue;
        }

#ifndef NDEBUG
        std::wcout << '\n';
        std::wcout << ansi_codes::light_blue << XORW(L"[F1] Pause\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[F2] Radar hack\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[F3] Glow hack\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[F4] ESP hack\n") << ansi_codes::reset_color;
        std::wcout << ansi_codes::light_blue << XORW(L"[INS] Minimize\n") << ansi_codes::reset_color;
        std::wcout << '\n';
#endif

        if (is_paused || !commons::window::isWindowInFocus(L"Counter-Strike 2"))
        {
            std::wcout << XORW(L"[F1] Paused...\n");
            draw_list.clear();
            sleep_for(50ms);
            continue;
        }

        const auto me{cheat.get_local_player(driver)};
        const auto my_team{me.get_team(driver)};
        const auto view_matrix{cheat.get_view_matrix(driver)};

#ifndef NDEBUG
        const auto my_eye_pos{ me.get_eye_pos(driver) };
        const auto my_forward_vec{ me.get_forward_vector(driver) };
        std::wcout << XORW(L"Forward vec: ") << my_forward_vec << '\n'
            << XORW(L"My eye position: ") << my_eye_pos << '\n'
            << XORW(L"View matrix: ") << view_matrix << '\n';

        std::wcout << '\n';
#endif

        std::vector<draw::Rect> draw_items;
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

            const auto entity_eyes_pos_screen{cheat::Cs2CheatController::world_to_screen(view_matrix, entity->get_eye_pos(driver))};
            const auto entity_feet_pos_screen{cheat::Cs2CheatController::world_to_screen(view_matrix, entity->get_vec_origin(driver))};

#ifndef NDEBUG
            std::wcout << XORW(L"Ent: ") << i << '\n'
                << XORW(L"Name: ") << entity->get_name(driver) << '\n'
                << XORW(L"Is this me: ") << (is_local_player ? "yes" : "no") << '\n'
                << XORW(L"Enemy: ") << (my_team != player_team ? "yes" : "no") << '\n'
                << XORW(L"HP: ") << std::dec << player_health << '\n'
                << XORW(L"Visible on Radar: ") << (entity_spotted ? "yes" : "no") << '\n'
                << XORW(L"w2s: ") << w2s_pos << '\n';
#endif

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

                if (esp_hack && entity_eyes_pos_screen.x > 0 && entity_eyes_pos_screen.x < 1920 && entity_eyes_pos_screen.y > 0 && entity_eyes_pos_screen.y < 1080)
                {
                    constexpr ImU32 color{IM_COL32(255, 0, 0, 255)};
                    draw_items.emplace_back(ImVec2{entity_eyes_pos_screen.x, entity_eyes_pos_screen.y}, ImVec2(50, 50), color);
                    draw_items.emplace_back(ImVec2{entity_feet_pos_screen.x, entity_feet_pos_screen.y}, ImVec2(50, 50), color);
                }
            }

#ifndef NDEBUG
            std::wcout << '\n';
#endif
        }

        draw_list.update(draw_items);
        sleep_for(1ms);
    }

    draw_thread.request_stop();

    return EXIT_SUCCESS;
}
