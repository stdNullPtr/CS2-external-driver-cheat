#pragma once
#include <cstdint>
#include "../imgui/lib/imgui.h"
#include "../sdk/dumper/client_dll.hpp"
#include "../driver/Driver.hpp"
#include "../util/Vec3.hpp"

namespace cheat::entity
{
    using namespace cs2_dumper::schemas;

    class Entity
    {
    private:
        uintptr_t entity_controller_;
        uintptr_t entity_pawn_;

    private:
        //TODO base classes for these classes maybe so we can move their specific field retrieval there
        [[nodiscard]] uintptr_t get_game_scene_node(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t get_movement_services(const driver::Driver& driver) const;

    public:
        Entity(const uintptr_t& entity_controller, const uintptr_t& entity_pawn);

        [[nodiscard]] std::string get_name(const driver::Driver& driver) const;
        [[nodiscard]] std::string get_weapon_name(const driver::Driver& driver) const;
        [[nodiscard]] int get_team(const driver::Driver& driver) const;
        [[nodiscard]] int get_health(const driver::Driver& driver) const;
        [[nodiscard]] bool is_spotted(const driver::Driver& driver) const;
        [[nodiscard]] bool is_local_player(const driver::Driver& driver) const;
        [[nodiscard]] bool is_glowing(const driver::Driver& driver) const;
        [[nodiscard]] bool is_scoped(const driver::Driver& driver) const;
        [[nodiscard]] bool is_flashed(const driver::Driver& driver) const;
        [[nodiscard]] util::Vec3 get_forward_vector(const driver::Driver& driver) const;
        [[nodiscard]] util::Vec3 get_vec_origin(const driver::Driver& driver) const;
        [[nodiscard]] util::Vec3 get_eye_pos(const driver::Driver& driver) const;
        [[nodiscard]] util::Vec3 get_head_bone_pos(const driver::Driver& driver) const;

        void set_spotted(const driver::Driver& driver, const bool& spotted) const;
        void disable_flash(const driver::Driver& driver) const;
        void set_glowing(const driver::Driver& driver, const bool& glow) const;
    };
}
