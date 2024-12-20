#pragma once
#include <stop_token>

#include "../global.hpp"

namespace cheat::aim
{
    using util::Vec2;
    using namespace std::chrono_literals;
    using std::this_thread::sleep_for;

    inline std::optional<Vec2> find_valid_aim_target_in_circle(const Vec2& center, const float& radius, const std::vector<Vec2>& aim_targets)
    {
        const float radius_squared{ radius * radius };
        float min_distance_squared{ std::numeric_limits<float>::max() };
        std::optional<Vec2> aim_target{ std::nullopt };

        for (const auto& target : aim_targets)
        {
            const float dist_squared{ center.distance_squared(target) };

            if (dist_squared <= radius_squared && dist_squared < min_distance_squared)
            {
                min_distance_squared = dist_squared;
                aim_target = target;
            }
        }

        return aim_target;
    }

    //TODO commons
    inline void move_mouse(const float& delta_x, const float& delta_y)
    {
        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi.dx = static_cast<LONG>(delta_x);
        input.mi.dy = static_cast<LONG>(delta_y);
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void aim_loop(const std::stop_token& stop_token, const std::optional<Vec2>& aim_target)
    {
        static const ImVec2 crosshair{ g::screen_center };

        while (!stop_token.stop_requested())
        {
            if (aim_target && g::toggles::aim_hack && !g::toggles::show_menu)
            {
                if (GetAsyncKeyState(VK_MBUTTON) & 0x8000 || (g::toggles::aim_assist && GetAsyncKeyState(VK_LBUTTON) & 0x8000))
                {
                    const float delta_x{ aim_target->x - crosshair.x };
                    const float delta_y{ aim_target->y - crosshair.y };

                    move_mouse(delta_x, delta_y);
                    sleep_for(10us);
                }
            }
            else
            {
                sleep_for(50ms);
            }
        }
    }
}
