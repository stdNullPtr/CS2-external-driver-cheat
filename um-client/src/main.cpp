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
#include <fcntl.h>

#include "controller/Aim.hpp"
#include "render/Render.hpp"
#include "util/Esp.hpp"

using namespace std::chrono_literals;
using namespace g::toggles;
using namespace commons::console;
using std::this_thread::sleep_for;
using util::Vec2;
using render::DrawCache;

int main()
{
    setCursorVisibility(false);

    const driver::Driver driver{};
    if (!driver.is_valid())
    {
        MessageBox(nullptr, XORW(L"Load the Driver first lmao."), XORW(L"RIP"), MB_OK);
        return EXIT_FAILURE;
    }

    cheat::Cs2CheatController cheat;

    render::EspDrawList draw_list;
    std::optional<Vec2> aim_target;

    std::jthread draw_thread(render::draw_scene, std::ref(draw_list));
    std::jthread aim_thread(cheat::aim::aim_loop, std::ref(aim_target));

    sleep_for(2s);

    while (!(GetAsyncKeyState(VK_END) & 0x1))
    {
#ifndef NDEBUG
        clearConsole({0, 0});
#endif
        if (!cheat.is_state_valid())
        {
            std::wcout << XORW(L"Controller state invalid, will re-init...\n");

            // prevent any leftover draw_scene artifacts
            draw_list.clear();
            reset_toggles();

            if (!commons::window::waitForWindow(XORW(L"Counter-Strike 2")))
            {
                std::wcout << XORW(L"Aborted looking for game window, exiting...\n");
                return EXIT_SUCCESS;
            }

            sleep_for(2s);

            if (!cheat.init(driver))
            {
                std::wcerr << XORW(L"Failed initializing cheat controller state, retrying...\n");
                sleep_for(2s);
                continue;
            }

            if (cheat.build_number_changed(driver))
            {
                MessageBox(nullptr, XORW(L"There was an update, update the client."), XORW(L"RIP"), MB_OK);
                return EXIT_FAILURE;
            }
        }

#ifndef NDEBUG
        std::wcout << ansi_codes::red << XORW(L"[END] Quit\n") << ansi_codes::reset_color;
#endif

        //TODO a better approach would be for EACH of these 'wait' functions to be in its own loop until the condition is met. Not to rely on the parent main loop to re-visit.
        if (!cheat.is_in_game(driver))
        {
            std::wcout << XORW(L"Waiting for you to join game...\n");

            // prevent any leftover draw_scene artifacts
            draw_list.clear();
            reset_toggles();

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

        if (is_paused || (!commons::window::isWindowInFocus(g::cs2_window_name) && !commons::window::isWindowInFocus(cheat::imgui::g::overlay_window_name)))
        {
            std::wcout << XORW(L"[F1] Paused...\n");
            draw_list.clear();
            sleep_for(50ms);
            continue;
        }

        const auto me{cheat.get_local_player(driver)};
        const auto my_team{me.get_team(driver)};
        const auto view_matrix{cheat.get_view_matrix(driver)};
        const auto c4_remaining_time{cheat.c4_blow_remaining_time(driver)};

        if (no_flash_hack && me.is_flashed(driver))
        {
            me.disable_flash(driver);
        }

        std::vector<DrawCache> draw_items;
        draw_items.reserve(32);

        if (esp_hack && c4_remaining_time > 0.0f)
        {
            const auto is_planted_a{cheat.c4_is_bomb_site_a(driver)};
            const auto esp_c4{util::esp::build_c4_esp(c4_remaining_time, is_planted_a)};
            draw_items.append_range(esp_c4);
        }

#ifndef NDEBUG
        const auto my_eye_pos{me.get_eye_pos(driver)};
        const auto my_forward_vec{me.get_forward_vector(driver)};
        std::wcout << XORW(L"Forward vec: ") << my_forward_vec << '\n'
            << XORW(L"My eye position: ") << my_eye_pos << '\n'
            << XORW(L"View matrix: ") << view_matrix << '\n';

        std::wcout << '\n';
#endif

        std::vector<Vec2> aim_targets;
        static int local_player_index{-1};
        for (int i{1}; i < 64; i++)
        {
            const std::optional entity{cheat.get_entity_from_list(driver, i)};

            if (!entity.has_value())
            {
                continue;
            }

            if (entity->is_local_player(driver))
            {
                local_player_index = i - 1;
            }

            const auto player_health{entity->get_health(driver)};
            const auto player_team{entity->get_team(driver)};
            const auto is_valid_enemy{my_team != player_team && player_health > 0};

            if (!is_valid_enemy)
            {
                continue;
            }

            const auto entity_spotted{entity->is_spotted(driver)};
            const auto entity_spotted_by_local_player{entity->is_spotted_by_local_player(driver, local_player_index)};
            const auto entity_name{entity->get_name(driver)};
            const auto is_scoped{entity->is_scoped(driver)};
            auto weapon_name{entity->get_weapon_name(driver)};
            if (weapon_name.length() > 7)
            {
                weapon_name = weapon_name.substr(7); //get rid of weapon_ prefix
            }

            const auto entity_eyes_pos_screen{render::utils::world_to_screen(view_matrix, entity->get_eye_pos(driver))};
            const auto entity_feet_pos_screen{render::utils::world_to_screen(view_matrix, entity->get_vec_origin(driver))};

#ifndef NDEBUG
            std::wcout << XORW(L"Ent: ") << i << '\n'
                << XORW(L"Enemy: ") << (my_team != player_team ? "yes" : "no") << '\n'
                << XORW(L"HP: ") << std::dec << player_health << '\n'
                << XORW(L"Visible on Radar: ") << (entity_spotted ? "yes" : "no") << '\n';
#endif

            const auto head_bone_pos_world{entity->get_head_bone_pos(driver)};
            const auto head_bone_pos_screen{render::utils::world_to_screen(view_matrix, head_bone_pos_world)};
            if (entity_spotted_by_local_player)
            {
                const auto aim_position{head_bone_pos_screen};
                aim_targets.emplace_back(aim_position);
            }

            if (glow_hack)
            {
                entity->set_glowing(driver, true);
            }

            if (radar_hack && !entity_spotted)
            {
                //TODO should we set the correct mask as well? we are setting this bool but the variables after it should be 1 as well
                entity->set_spotted(driver, true);
            }

            if (esp_hack && (render::utils::is_in_screen(entity_eyes_pos_screen) || render::utils::is_in_screen(entity_feet_pos_screen)))
            {
                const auto esp_box_color{entity_spotted_by_local_player ? g::esp_color_enemy_visible : g::esp_color};

                const auto esp_player{util::esp::build_player_esp(entity_eyes_pos_screen, entity_feet_pos_screen, esp_box_color)};
                const auto esp_bones{util::esp::build_bone_esp(head_bone_pos_screen)};
                const auto esp_health{util::esp::build_health_esp(esp_player.get_top_left(), esp_player.get_bottom_right(), player_health, esp_box_color)};
                const auto esp_player_bottom{util::esp::build_player_bottom_esp(entity_name, entity_eyes_pos_screen, esp_player.get_bottom_right(), weapon_name)};
                const auto esp_player_top{util::esp::build_player_top_esp(is_scoped, entity_eyes_pos_screen, esp_player.get_top_left())};

                draw_items.emplace_back(esp_player);
                draw_items.emplace_back(esp_bones);
                draw_items.append_range(esp_health);
                draw_items.append_range(esp_player_bottom);
                draw_items.append_range(esp_player_top);
            }

#ifndef NDEBUG
            std::wcout << '\n';
#endif
        }

        if (aim_hack)
        {
            aim_target = cheat::aim::find_valid_aim_target_in_circle(Vec2{.x = g::screen_center.x, .y = g::screen_center.y}, g::aim_fov, aim_targets);
        }

        draw_list.update(draw_items);
        sleep_for(1ms);
    }

    draw_thread.request_stop();
    aim_thread.request_stop();

    return EXIT_SUCCESS;
}
