#pragma once
#include <chrono>
#include <process.hpp>
#include <optional>

#include "../global.hpp"
#include "../driver/Driver.hpp"
#include "../sdk/dumper/client_dll.hpp"
#include "../sdk/dumper/offsets.hpp"
#include "Entity.hpp"
#include "../util/Vec3.hpp"
#include "../util/Vec2.hpp"
#include "../util/ViewMatrix.hpp"
#include "../render/Render.hpp"

namespace cheat
{
    class Cs2CheatController
    {
    private:
        DWORD cs2_process_id_{0};
        uintptr_t client_dll_base_{0};
        uintptr_t engine_dll_base_{0};
        uintptr_t entity_system_{0};

    private:
        [[nodiscard]] static std::optional<DWORD> get_cs2_process_id();

        [[nodiscard]] std::optional<uintptr_t> find_client_dll_base() const;
        [[nodiscard]] std::optional<uintptr_t> find_engine_dll_base() const;
        [[nodiscard]] std::optional<uintptr_t> find_entity_system(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t find_local_player_controller(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t find_local_player_pawn(const driver::Driver& driver) const;

        [[nodiscard]] bool attach(const driver::Driver& driver) const;

    public:
        Cs2CheatController() = default;
        ~Cs2CheatController() = default;

        Cs2CheatController(const Cs2CheatController& other) = delete;
        Cs2CheatController(Cs2CheatController&& other) noexcept = delete;
        Cs2CheatController& operator=(const Cs2CheatController& other) = delete;
        Cs2CheatController& operator=(Cs2CheatController&& other) noexcept = delete;

        [[nodiscard]] uintptr_t get_network_client(const driver::Driver& driver) const;
        [[nodiscard]] bool is_in_game(const driver::Driver& driver) const;
        [[nodiscard]] bool is_state_valid() const;

        [[nodiscard]] std::optional<entity::Entity> get_entity_from_list(const driver::Driver& driver, const int& index) const;
        [[nodiscard]] entity::Entity get_local_player(const driver::Driver& driver) const;
        [[nodiscard]] util::ViewMatrix get_view_matrix(const driver::Driver& driver) const;

        [[nodiscard]] float c4_blow_remaining_time(const driver::Driver& driver) const;
        [[nodiscard]] bool c4_is_bomb_site_a(const driver::Driver& driver) const;

        [[nodiscard]] bool init(const driver::Driver& driver);

        [[nodiscard]] std::optional<uintptr_t> get_entity_controller(const driver::Driver& driver, const int& i) const;
        [[nodiscard]] std::optional<uintptr_t> get_entity_pawn(const driver::Driver& driver, const uintptr_t& entity_controller) const;
    };
}
