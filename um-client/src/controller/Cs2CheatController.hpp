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
#include "../sdk/C_CSPlayerPawn.hpp"
#include "../sdk/CCSPlayerController.hpp"

namespace cheat
{
    using namespace sdk;
    using driver::Driver;
    using entity::Entity;
    using util::ViewMatrix;

    class Cs2CheatController
    {
        DWORD cs2_process_id_{0};
        uintptr_t client_dll_base_{0};
        uintptr_t engine_dll_base_{0};
        uintptr_t entity_system_{0};

        [[nodiscard]] static std::optional<DWORD> get_cs2_process_id();

        [[nodiscard]] std::optional<uintptr_t> find_client_dll_base() const;
        [[nodiscard]] std::optional<uintptr_t> find_engine_dll_base() const;
        [[nodiscard]] std::optional<uintptr_t> find_entity_system(const Driver& driver) const;
        [[nodiscard]] uintptr_t find_local_player_controller(const Driver& driver) const;
        [[nodiscard]] uintptr_t find_local_player_pawn(const Driver& driver) const;
        [[nodiscard]] std::optional<CCSPlayerController> get_entity_controller(const Driver& driver, const int& i) const;
        [[nodiscard]] std::optional<C_CSPlayerPawn> get_entity_pawn(const Driver& driver, const CCSPlayerController& entity_controller) const;

        [[nodiscard]] C_CSPlayerPawn get_local_player_pawn(const Driver& driver) const;
        [[nodiscard]] CCSPlayerController get_local_player_controller(const Driver& driver) const;

        [[nodiscard]] uintptr_t get_network_client(const Driver& driver) const;

        [[nodiscard]] bool attach(const Driver& driver) const;

    public:
        Cs2CheatController() = default;
        ~Cs2CheatController() = default;

        Cs2CheatController(const Cs2CheatController& other) = delete;
        Cs2CheatController(Cs2CheatController&& other) noexcept = delete;
        Cs2CheatController& operator=(const Cs2CheatController& other) = delete;
        Cs2CheatController& operator=(Cs2CheatController&& other) noexcept = delete;

        [[nodiscard]] bool is_in_game(const Driver& driver) const;
        [[nodiscard]] bool is_state_valid() const;

        [[nodiscard]] std::optional<Entity> get_entity_from_list(const Driver& driver, const int& index) const;
        [[nodiscard]] Entity get_local_player_entity(const Driver& driver) const;
        [[nodiscard]] ViewMatrix get_view_matrix(const Driver& driver) const;

        [[nodiscard]] float c4_blow_remaining_time(const Driver& driver) const;
        [[nodiscard]] bool c4_is_bomb_site_a(const Driver& driver) const;

        [[nodiscard]] bool init(const Driver& driver);
    };
}
