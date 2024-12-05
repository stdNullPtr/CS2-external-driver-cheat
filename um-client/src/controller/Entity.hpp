#pragma once
#include <cstdint>
#include "../imgui/lib/imgui.h"
#include "../sdk/dumper/client_dll.hpp"
#include "../driver/Driver.hpp"

namespace cheat::entity
{
    using namespace cs2_dumper::schemas;

    struct Vec3
    {
        float x, y, z;

        Vec3 operator+(const Vec3& other) const
        {
            return {.x = x + other.x, .y = y + other.y, .z = z + other.z};
        }

        Vec3 operator-(const Vec3& other) const
        {
            return {.x = x - other.x, .y = y - other.y, .z = z - other.z};
        }

        friend std::ostream& operator<<(std::ostream& os, const Vec3& vec)
        {
            os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
            return os;
        }
    };

    class Entity
    {
    private:
        const uintptr_t m_entity_controller;
        const uintptr_t m_entity_pawn;

    private:
        //TODO base classes for these classes maybe so we can move their specific field retrieval there
        [[nodiscard]] uintptr_t get_game_scene_node(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t get_movement_services(const driver::Driver& driver) const;

    public:
        Entity(uintptr_t entity_controller, uintptr_t entity_pawn);
        ~Entity() = default;

        Entity(Entity&& other) noexcept = default;
        Entity(const Entity& other) = default;
        Entity& operator=(const Entity& other) = delete;
        Entity& operator=(Entity&& other) noexcept = delete;

        [[nodiscard]] std::string get_name(const driver::Driver& driver) const;

        [[nodiscard]] int get_team(const driver::Driver& driver) const;

        [[nodiscard]] int get_health(const driver::Driver& driver) const;

        [[nodiscard]] bool is_spotted(const driver::Driver& driver) const;

        [[nodiscard]] bool is_local_player(const driver::Driver& driver) const;

        [[nodiscard]] bool is_glowing(const driver::Driver& driver) const;

        [[nodiscard]] Vec3 get_forward_vector(const driver::Driver& driver) const;

        [[nodiscard]] Vec3 get_vec_origin(const driver::Driver& driver) const;

        [[nodiscard]] Vec3 get_eye_pos(const driver::Driver& driver) const;

        void set_spotted(const driver::Driver& driver, const bool& spotted) const;

        void set_glowing(const driver::Driver& driver, const bool& glow) const;
    };
}
