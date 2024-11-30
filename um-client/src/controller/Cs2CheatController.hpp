#pragma once
#include <process.hpp>

#include "../global.hpp"
#include "../driver/driver.hpp"
#include "../sdk/clientDll.hpp"
#include "../sdk/offsets.hpp"

namespace cheat_controller
{
    class Cs2CheatController
    {
    public:
        const driver::Driver m_driver;
        const DWORD m_cs2_process_id;
        const uintptr_t m_client_dll_base;

    private:
        static DWORD getCs2ProcessId();
        uintptr_t getClientDllBase() const;

    public:
        Cs2CheatController();
        ~Cs2CheatController() = default;
        uintptr_t get_local_player_controller() const;
        uintptr_t get_local_player_pawn() const;

        Cs2CheatController(const Cs2CheatController& other) = delete;
        Cs2CheatController(Cs2CheatController&& other) noexcept = delete;
        Cs2CheatController& operator=(const Cs2CheatController& other) = delete;
        Cs2CheatController& operator=(Cs2CheatController&& other) noexcept = delete;

        std::optional<uintptr_t> get_entity_controller(const uintptr_t& entity_system, const int& i) const;
        std::optional<uintptr_t> get_entity_pawn(const uintptr_t& entity_system,
                                                 const uintptr_t& entity_controller) const;
    };
}
