#include "Entity.hpp"

namespace cheat::entity
{
    uintptr_t Entity::get_game_scene_node(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(m_entity_pawn + client_dll::C_BaseEntity::m_pGameSceneNode);
    }

    uintptr_t Entity::get_movement_services(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(m_entity_pawn + client_dll::C_BasePlayerPawn::m_pMovementServices);
    }

    Entity::Entity(const uintptr_t entity_controller, const uintptr_t entity_pawn): m_entity_controller(entity_controller), m_entity_pawn(entity_pawn)
    {
    }

    std::string Entity::get_name(const driver::Driver& driver) const
    {
        const auto entity_name_address{driver.read<uintptr_t>(m_entity_controller + client_dll::CCSPlayerController::m_sSanitizedPlayerName)};
        struct str_wrap
        {
            char buf[20];
        };
        const auto str{driver.read<str_wrap>(entity_name_address)};

        return {str.buf};
    }

    int Entity::get_team(const driver::Driver& driver) const
    {
        return driver.read<int>(m_entity_pawn + client_dll::C_BaseEntity::m_iTeamNum);
    }

    int Entity::get_health(const driver::Driver& driver) const
    {
        return driver.read<int>(m_entity_pawn + client_dll::C_BaseEntity::m_iHealth);
    }

    bool Entity::is_spotted(const driver::Driver& driver) const
    {
        const auto p_entity_spotted_state{m_entity_pawn + client_dll::C_CSPlayerPawn::m_entitySpottedState};

        const auto entity_spotted_address{p_entity_spotted_state + client_dll::EntitySpottedState_t::m_bSpotted};

        return driver.read<bool>(entity_spotted_address);
    }

    bool Entity::is_local_player(const driver::Driver& driver) const
    {
        return driver.read<bool>(m_entity_controller + client_dll::CBasePlayerController::m_bIsLocalPlayerController);
    }

    bool Entity::is_glowing(const driver::Driver& driver) const
    {
        return driver.read<bool>(m_entity_pawn + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_bGlowing);
    }

    Vec3 Entity::get_forward_vector(const driver::Driver& driver) const
    {
        const auto movement_services{get_movement_services(driver)};
        return driver.read<Vec3>(movement_services + client_dll::CCSPlayer_MovementServices::m_vecForward);
    }

    Vec3 Entity::get_vec_origin(const driver::Driver& driver) const
    {
        return driver.read<Vec3>(get_game_scene_node(driver) + client_dll::CGameSceneNode::m_vecOrigin);
    }

    Vec3 Entity::get_eye_pos(const driver::Driver& driver) const
    {
        return driver.read<Vec3>(m_entity_pawn + client_dll::C_BaseModelEntity::m_vecViewOffset) + get_vec_origin(driver);
    }

    void Entity::set_spotted(const driver::Driver& driver, const bool& spotted) const
    {
        const auto p_entity_spotted_state{m_entity_pawn + client_dll::C_CSPlayerPawn::m_entitySpottedState};

        const auto entity_spotted_address{p_entity_spotted_state + client_dll::EntitySpottedState_t::m_bSpotted};

        driver.write(entity_spotted_address, spotted);
    }

    void Entity::set_glowing(const driver::Driver& driver, const bool& glow) const
    {
        driver.write(m_entity_pawn + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_bGlowing, glow);
        if (glow)
        {
            driver.write(m_entity_pawn + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_iGlowType, 3);
            driver.write(m_entity_pawn + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_glowColorOverride, IM_COL32(245, 0, 0, 255));
        }
    }
}