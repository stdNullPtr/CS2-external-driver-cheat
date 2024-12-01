#include "EntityController.hpp"

namespace cheat::entity
{
    EntityController::EntityController(const driver::Driver& driver, const uintptr_t entity_controller, const uintptr_t entity_pawn): m_driver(driver), m_entityController(entity_controller), m_entityPawn(entity_pawn)
    {
    }

    std::string EntityController::get_name() const
    {
        const auto entityNameAddress = m_driver.read<uintptr_t>(m_entityController + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
        struct strWrap
        {
            char buf[20];
        };
        const auto str{m_driver.read<strWrap>(entityNameAddress)};

        return std::string(str.buf);
    }

    int EntityController::get_team() const
    {
        return m_driver.read<int>(m_entityPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
    }

    int EntityController::get_health() const
    {
        return m_driver.read<int>(m_entityPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
    }

    bool EntityController::is_spotted() const
    {
        const auto pEntitySpottedState{m_entityPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState};

        const auto entitySpottedAddress{pEntitySpottedState + cs2_dumper::schemas::client_dll::EntitySpottedState_t::m_bSpotted};

        return m_driver.read<bool>(entitySpottedAddress);
    }

    bool EntityController::is_local_player() const
    {
        return m_driver.read<bool>(m_entityController + cs2_dumper::schemas::client_dll::CBasePlayerController::m_bIsLocalPlayerController);
    }

    void EntityController::set_spotted(const bool& new_spotted_state) const
    {
        const auto pEntitySpottedState{m_entityPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState};

        const auto entitySpottedAddress{pEntitySpottedState + cs2_dumper::schemas::client_dll::EntitySpottedState_t::m_bSpotted};

        m_driver.write(entitySpottedAddress, new_spotted_state);
    }
}
