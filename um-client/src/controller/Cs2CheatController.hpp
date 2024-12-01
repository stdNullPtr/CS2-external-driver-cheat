#pragma once
#include <chrono>
#include <process.hpp>

#include "../global.hpp"
#include "../driver/Driver.hpp"
#include "../sdk/dumper/client_dll.hpp"
#include "../sdk/dumper/offsets.hpp"

namespace cheat
{
    class Cs2CheatController
    {
    public:
        const driver::Driver& m_driver;

    private:
        DWORD m_cs2_process_id{0};
        uintptr_t m_client_dll_base{0};
        uintptr_t m_engine_dll_base{0};

    private:
        static DWORD get_cs2_process_id();
        [[nodiscard]] uintptr_t find_client_dll_base() const;
        [[nodiscard]] uintptr_t find_engine_dll_base() const;
        [[nodiscard]] bool attach() const;

    public:
        Cs2CheatController(const driver::Driver& driver);
        ~Cs2CheatController() = default;

        Cs2CheatController(const Cs2CheatController& other) = delete;
        Cs2CheatController(Cs2CheatController&& other) noexcept = delete;
        Cs2CheatController& operator=(const Cs2CheatController& other) = delete;
        Cs2CheatController& operator=(Cs2CheatController&& other) noexcept = delete;

        [[nodiscard]] DWORD get_m_cs2_process_id() const;
        [[nodiscard]] uintptr_t get_client_dll_base() const;
        [[nodiscard]] uintptr_t get_engine_dll_base() const;

        [[nodiscard]] uintptr_t get_local_player_controller() const;
        [[nodiscard]] uintptr_t get_local_player_pawn() const;
        [[nodiscard]] uintptr_t get_network_client() const;
        [[nodiscard]] bool is_in_game() const;
        [[nodiscard]] bool is_state_valid() const;

        [[nodiscard]] bool init();

        [[nodiscard]] std::optional<uintptr_t> get_entity_controller(const uintptr_t& entity_system, const int& i) const;
        [[nodiscard]] std::optional<uintptr_t> get_entity_pawn(const uintptr_t& entity_system, const uintptr_t& entity_controller) const;
    };
}
