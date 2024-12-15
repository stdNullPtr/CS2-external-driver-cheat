#pragma once
#include <stop_token>

namespace cheat::aim
{
    using util::Vec2;

    inline void moveMouse(float deltaX, float deltaY)
    {
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dx = static_cast<LONG>(deltaX);
        input.mi.dy = static_cast<LONG>(deltaY);
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void aimLoop(const std::stop_token& stop_token, const std::optional<Vec2>& aim_target)
    {
        static const Vec2 crosshair{g::screen_width / 2.0f, g::screen_height / 2.0f};

        while (!stop_token.stop_requested())
        {
            if (aim_target && g::toggles::aim_hack && (GetAsyncKeyState(VK_MBUTTON) & 0x8000))
            {
                float deltaX = aim_target->x - crosshair.x;
                float deltaY = aim_target->y - crosshair.y;

                moveMouse(deltaX, deltaY);
            }

            //todo remove when ready
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    }
}
