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
        uintptr_t entity_controller_;
        uintptr_t entity_pawn_;

        std::string name_;
        int team_;
        int hp_;
        bool is_spotted_;
        bool is_local_player_;
        bool is_glowing_;
        Vec3 forward_vec_;
        Vec3 origin_;
        Vec3 eye_position_;

    private:
        //TODO base classes for these classes maybe so we can move their specific field retrieval there
        [[nodiscard]] uintptr_t get_game_scene_node(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t get_movement_services(const driver::Driver& driver) const;

    public:
        Entity(const uintptr_t& entity_controller, const uintptr_t& entity_pawn);
        Entity(const driver::Driver& driver, const uintptr_t& entity_controller, const uintptr_t& entity_pawn);

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
