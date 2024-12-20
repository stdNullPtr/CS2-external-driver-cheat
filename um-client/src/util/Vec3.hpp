#pragma once
#include <ostream>

namespace util
{
    struct Vec3
    {
        float x, y, z;

        Vec3 operator+(const Vec3& other) const
        {
            return { .x = x + other.x, .y = y + other.y, .z = z + other.z };
        }

        Vec3 operator-(const Vec3& other) const
        {
            return { .x = x - other.x, .y = y - other.y, .z = z - other.z };
        }

        friend std::ostream& operator<<(std::ostream& os, const Vec3& vec)
        {
            os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
            return os;
        }

        friend std::wostream& operator<<(std::wostream& os, const Vec3& vec)
        {
            os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
            return os;
        }
    };
}
