#pragma once
#include <process.hpp>

#include "../global.hpp"
#include "../driver/driver.hpp"
#include "../sdk/clientDll.hpp"
#include "../sdk/offsets.hpp"

namespace cheat_controller
{
    class cs2_cheat_controller
    {
    public:
        const driver::driver m_driver;
        const DWORD m_cs2_process_id;
        const uintptr_t m_client_dll_base;

    private:
        static DWORD getCs2ProcessId();
        [[nodiscard]] uintptr_t getClientDllBase() const;

    public:
        cs2_cheat_controller();
        ~cs2_cheat_controller() = default;

        cs2_cheat_controller(const cs2_cheat_controller& other) = delete;
        cs2_cheat_controller(cs2_cheat_controller&& other) noexcept = delete;
        cs2_cheat_controller& operator=(const cs2_cheat_controller& other) = delete;
        cs2_cheat_controller& operator=(cs2_cheat_controller&& other) noexcept = delete;

        [[nodiscard]] uintptr_t get_local_player_controller() const;
        [[nodiscard]] uintptr_t get_local_player_pawn() const;

        [[nodiscard]] std::optional<uintptr_t> get_entity_controller(const uintptr_t& entity_system, const int& i) const;
        [[nodiscard]] std::optional<uintptr_t> get_entity_pawn(const uintptr_t& entity_system, const uintptr_t& entity_controller) const;

        int get_highest_entity_index(const uintptr_t& entity_system) const;
    };
}
