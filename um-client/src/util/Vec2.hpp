#pragma once
#include <ostream>

namespace util
{
    struct Vec2
    {
        float x, y;

        [[nodiscard]] float distance_squared(const Vec2& other) const
        {
            const float dx{ x - other.x };
            const float dy{ y - other.y };
            return dx * dx + dy * dy;
        }

        Vec2 operator+(const Vec2& other) const
        {
            return { .x = x + other.x, .y = y + other.y };
        }

        Vec2 operator-(const Vec2& other) const
        {
            return { .x = x - other.x, .y = y - other.y };
        }

        friend std::ostream& operator<<(std::ostream& os, const Vec2& vec)
        {
            os << "[" << vec.x << ", " << vec.y << "]";
            return os;
        }

        friend std::wostream& operator<<(std::wostream& os, const Vec2& vec)
        {
            os << L"[" << vec.x << L", " << vec.y << L"]";
            return os;
        }
    };
}
