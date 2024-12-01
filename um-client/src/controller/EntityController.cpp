#include "EntityController.hpp"

namespace cheat::entity
{
    EntityController::EntityController(const driver::Driver& driver, const uintptr_t entity_controller, const uintptr_t entity_pawn): m_driver(driver), m_entity_controller(entity_controller), m_entity_pawn(entity_pawn)
    {
    }

    std::string EntityController::get_name() const
    {
        const auto entity_name_address{m_driver.read<uintptr_t>(m_entity_controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName)};
        struct str_wrap
        {
            char buf[20];
        };
        const auto str{m_driver.read<str_wrap>(entity_name_address)};

        return {str.buf};
    }

    int EntityController::get_team() const
    {
        return m_driver.read<int>(m_entity_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
    }

    int EntityController::get_health() const
    {
        return m_driver.read<int>(m_entity_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
    }

    bool EntityController::is_spotted() const
    {
        const auto p_entity_spotted_state{m_entity_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState};

        const auto entity_spotted_address{p_entity_spotted_state + cs2_dumper::schemas::client_dll::EntitySpottedState_t::m_bSpotted};

        return m_driver.read<bool>(entity_spotted_address);
    }

    bool EntityController::is_local_player() const
    {
        return m_driver.read<bool>(m_entity_controller + cs2_dumper::schemas::client_dll::CBasePlayerController::m_bIsLocalPlayerController);
    }

    void EntityController::set_spotted(const bool& new_spotted_state) const
    {
        const auto p_entity_spotted_state{m_entity_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState};

        const auto entity_spotted_address{p_entity_spotted_state + cs2_dumper::schemas::client_dll::EntitySpottedState_t::m_bSpotted};

        m_driver.write(entity_spotted_address, new_spotted_state);
    }
}
