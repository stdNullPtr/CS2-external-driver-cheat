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
#include <io.h>
#include <fcntl.h>

#include "controller/Aim.hpp"
#include "render/Render.hpp"

using namespace std::chrono_literals;
using namespace g::toggles;
using namespace commons::console;
using std::this_thread::sleep_for;
using util::Vec2;


std::optional<Vec2> findValidAimTargetInCircle(const Vec2& center, const float& radius, const std::vector<Vec2>& entities)
{
    float radiusSquared = radius * radius;
    float minDistanceSquared = std::numeric_limits<float>::max();
    std::optional<Vec2> closestEntity = std::nullopt;

    for (const auto& entity : entities)
    {
        float distSquared = center.distanceSquared(entity);

        if (distSquared <= radiusSquared && distSquared < minDistanceSquared)
        {
            minDistanceSquared = distSquared;
            closestEntity = entity;
        }
    }

    return closestEntity;
}


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

    render::EspDrawList draw_list;
    std::optional<Vec2> aimTarget;

    std::jthread draw_thread(render::draw_scene, std::ref(draw_list));
    std::jthread aim_thread(cheat::aim::aimLoop, std::ref(aimTarget));

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

        if (is_paused || (!commons::window::isWindowInFocus(g::CS2_WINDOW_NAME) && !commons::window::isWindowInFocus(cheat::imgui::g::overlay_window_name)))
        {
            std::wcout << XORW(L"[F1] Paused...\n");
            draw_list.clear();
            sleep_for(50ms);
            continue;
        }

        const auto me{cheat.get_local_player(driver)};
        const auto my_team{me.get_team(driver)};
        const auto view_matrix{cheat.get_view_matrix(driver)};
        const auto c4RemainingTime{cheat.c4_blow_remaining_time(driver)};

        std::vector<render::DrawCache> draw_items;

        if (esp_hack && c4RemainingTime > 0.0f)
        {
            const auto c4TimerText{
                render::DrawCache::build_text(
                    std::string{XOR("C4 blow: ")}.append(std::to_string(c4RemainingTime)),
                    ImVec2{static_cast<float>(g::additionalScreenInfoPositionX), static_cast<float>(g::additionalScreenInfoPositionY)},
                    g::additionalScreenInfoTextColor,
                    0)
            };

            const auto isA{cheat.c4_is_bomb_site_a(driver)};
            const auto c4BombSiteText{
                render::DrawCache::build_text(
                    std::string{XOR("Plant site: ")}.append(isA ? "A" : "B"),
                    ImVec2{static_cast<float>(g::additionalScreenInfoPositionX), static_cast<float>(g::additionalScreenInfoPositionY)},
                    g::additionalScreenInfoTextColor,
                    1)
            };

            draw_items.emplace_back(c4TimerText);
            draw_items.emplace_back(c4BombSiteText);
        }

#ifndef NDEBUG
        const auto my_eye_pos{me.get_eye_pos(driver)};
        const auto my_forward_vec{me.get_forward_vector(driver)};
        std::wcout << XORW(L"Forward vec: ") << my_forward_vec << '\n'
            << XORW(L"My eye position: ") << my_eye_pos << '\n'
            << XORW(L"View matrix: ") << view_matrix << '\n';

        std::wcout << '\n';
#endif

        std::vector<Vec2> entities;
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
            const auto entity_name{entity->get_name(driver)};
            const auto is_scoped{entity->is_scoped(driver)};
            auto weaponName{entity->get_weapon_name(driver)};
            if (weaponName.length() > 7)
            {
                weaponName = weaponName.substr(7); //get rid of weapon_ prefix
            }

            const auto entity_eyes_pos_screen{render::world_to_screen(view_matrix, entity->get_eye_pos(driver))};
            const auto entity_feet_pos_screen{render::world_to_screen(view_matrix, entity->get_vec_origin(driver))};
            if (my_team != player_team && player_health > 0 && !is_local_player)
            {
                entities.emplace_back(entity_eyes_pos_screen);
            }

#ifndef NDEBUG
            std::wcout << XORW(L"Ent: ") << i << '\n'
                << XORW(L"Is this me: ") << (is_local_player ? "yes" : "no") << '\n'
                << XORW(L"Enemy: ") << (my_team != player_team ? "yes" : "no") << '\n'
                << XORW(L"HP: ") << std::dec << player_health << '\n'
                << XORW(L"Visible on Radar: ") << (entity_spotted ? "yes" : "no") << '\n';
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

                if (esp_hack && entity_eyes_pos_screen.x > 0
                    && entity_eyes_pos_screen.x < static_cast<float>(g::screen_width)
                    && entity_eyes_pos_screen.y > 0
                    && entity_eyes_pos_screen.y < static_cast<float>(g::screen_height))
                {
                    const auto espBoxColor{g::espColor};
                    const auto espHealthColor{g::espHealthColor};
                    const auto espBoxThickness{g::espBoxThickness};

                    constexpr float widthShrinkCoefficient{0.35f};
                    constexpr float heightShrinkCoefficient{0.15f};

                    const float widthRelativeToPlayerDistance{(entity_feet_pos_screen.y - entity_eyes_pos_screen.y) * widthShrinkCoefficient};
                    const float heightRelativeToPlayerDistance{(entity_feet_pos_screen.y - entity_eyes_pos_screen.y) * heightShrinkCoefficient};

                    const auto mainEspTopLeft{ImVec2{entity_eyes_pos_screen.x - widthRelativeToPlayerDistance, entity_eyes_pos_screen.y - heightRelativeToPlayerDistance}};
                    const auto mainEspBotRight{ImVec2{entity_feet_pos_screen.x + widthRelativeToPlayerDistance, entity_feet_pos_screen.y + heightRelativeToPlayerDistance}};
                    const auto espBox{render::DrawCache::build_rect(mainEspTopLeft, mainEspBotRight, false, espBoxColor, espBoxThickness)};

                    const auto healthBox{render::DrawCache::build_rect(ImVec2{mainEspTopLeft.x - 10.0f, mainEspTopLeft.y}, ImVec2{mainEspTopLeft.x, mainEspBotRight.y}, false, espBoxColor, espBoxThickness)};

                    float healthBarHeight = healthBox.get_bottom_right().y - healthBox.get_top_left().y;
                    float topLeftY = healthBox.get_bottom_right().y - (healthBarHeight * (static_cast<float>(player_health) / 100.0f));
                    const auto healthBoxFilled{
                        render::DrawCache::build_rect(
                            ImVec2{
                                healthBox.get_top_left().x + healthBox.get_thickness(),
                                topLeftY + healthBox.get_thickness()
                            },
                            {
                                healthBox.get_bottom_right().x - healthBox.get_thickness(),
                                healthBox.get_bottom_right().y - healthBox.get_thickness()
                            },
                            true,
                            espHealthColor,
                            espBoxThickness
                        )
                    };

                    const auto healthText{
                        render::DrawCache::build_text(
                            std::to_string(player_health).append("%"),
                            {
                                healthBox.get_top_left().x + cheat::imgui::g::font_size / 2.0f,
                                healthBox.get_top_left().y - cheat::imgui::g::font_size
                            },
                            espHealthColor,
                            0
                        )
                    };

                    const auto bottomTextColor{g::textColor};
                    const auto entityNameRenderObj{
                        render::DrawCache::build_text(
                            entity_name,
                            ImVec2{entity_eyes_pos_screen.x, espBox.get_bottom_right().y},
                            bottomTextColor,
                            0)
                    };

                    ImVec4 weaponTextColor{bottomTextColor};
                    if (weaponName.starts_with(XOR("awp")))
                    {
                        weaponTextColor = g::weaponAwpTextColor;
                    }
                    else if (weaponName.starts_with("knife"))
                    {
                        weaponTextColor = g::weaponKnifeTextColor;
                    }
                    const auto weaponNameNameRenderObj{
                        render::DrawCache::build_text(
                            weaponName,
                            ImVec2{entity_eyes_pos_screen.x, espBox.get_bottom_right().y},
                            weaponTextColor,
                            1)
                    };

                    draw_items.emplace_back(espBox);
                    draw_items.emplace_back(healthBox);
                    draw_items.emplace_back(healthBoxFilled);
                    draw_items.emplace_back(healthText);
                    draw_items.emplace_back(weaponNameNameRenderObj);
                    draw_items.emplace_back(entityNameRenderObj);

                    if (is_scoped)
                    {
                        const auto scopedText{
                            render::DrawCache::build_text(
                                ICON_FA_EXCLAMATION_TRIANGLE " >SCOPED< " ICON_FA_EXCLAMATION_TRIANGLE,
                                ImVec2{entity_eyes_pos_screen.x, espBox.get_top_left().y - cheat::imgui::g::font_size},
                                g::weaponAwpTextColor,
                                0)
                        };
                        draw_items.emplace_back(scopedText);
                    }
                }
            }

#ifndef NDEBUG
            std::wcout << '\n';
#endif
        }

        if (aim_hack)
        {
            aimTarget = findValidAimTargetInCircle(Vec2{g::screen_width / 2.0f, g::screen_height / 2.0f}, g::aimFov, entities);
        }

        draw_list.update(draw_items);
        sleep_for(1ms);
    }

    draw_thread.request_stop();
    aim_thread.request_stop();

    return EXIT_SUCCESS;
}
