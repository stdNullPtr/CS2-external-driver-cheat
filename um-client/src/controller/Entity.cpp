#include "Entity.hpp"

#include "../sdk/dumper/offsets.hpp"
#include "../util/BoneMatrix.hpp"

namespace cheat::entity
{
    uintptr_t Entity::get_game_scene_node(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(entity_pawn_ + client_dll::C_BaseEntity::m_pGameSceneNode);
    }

    uintptr_t Entity::get_movement_services(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(entity_pawn_ + client_dll::C_BasePlayerPawn::m_pMovementServices);
    }

    Entity::Entity(const uintptr_t& entity_controller, const uintptr_t& entity_pawn) : entity_controller_(entity_controller), entity_pawn_(entity_pawn)
    {
    }

    std::string Entity::get_name(const driver::Driver& driver) const
    {
        const auto entity_name_address = driver.read<uintptr_t>(
            entity_controller_ + client_dll::CCSPlayerController::m_sSanitizedPlayerName
        );

        struct str_wrap
        {
            char buf[20];
        };

        const auto str{driver.read<str_wrap>(entity_name_address)};

        return {str.buf, str.buf + strnlen(str.buf, sizeof(str.buf))};
    }

    std::string Entity::get_weapon_name(const driver::Driver& driver) const
    {
        struct str_wrap
        {
            char buf[20];
        };

        const auto weapon_base{driver.read<uintptr_t>(entity_pawn_ + client_dll::C_CSPlayerPawnBase::m_pClippingWeapon)};
        const auto identity{driver.read<uintptr_t>(weapon_base + client_dll::CEntityInstance::m_pEntity)};
        const auto designer_name{driver.read<uintptr_t>(identity + client_dll::CEntityIdentity::m_designerName)};
        const auto str{driver.read<str_wrap>(designer_name)};

        return {str.buf, str.buf + strnlen(str.buf, sizeof(str.buf))};
    }

    int Entity::get_team(const driver::Driver& driver) const
    {
        return driver.read<int>(entity_pawn_ + client_dll::C_BaseEntity::m_iTeamNum);
    }

    int Entity::get_health(const driver::Driver& driver) const
    {
        return driver.read<int>(entity_pawn_ + client_dll::C_BaseEntity::m_iHealth);
    }

    bool Entity::is_spotted(const driver::Driver& driver) const
    {
        const auto p_entity_spotted_state{entity_pawn_ + client_dll::C_CSPlayerPawn::m_entitySpottedState};
        const auto entity_spotted_address{p_entity_spotted_state + client_dll::EntitySpottedState_t::m_bSpotted};
        return driver.read<bool>(entity_spotted_address);
    }

    bool Entity::is_local_player(const driver::Driver& driver) const
    {
        return driver.read<bool>(entity_controller_ + client_dll::CBasePlayerController::m_bIsLocalPlayerController);
    }

    bool Entity::is_glowing(const driver::Driver& driver) const
    {
        return driver.read<bool>(entity_pawn_ + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_bGlowing);
    }

    bool Entity::is_scoped(const driver::Driver& driver) const
    {
        return driver.read<bool>(entity_pawn_ + client_dll::C_CSPlayerPawn::m_bIsScoped);
    }

    bool Entity::is_flashed(const driver::Driver& driver) const
    {
        return driver.read<float>(entity_pawn_ + client_dll::C_CSPlayerPawnBase::m_flFlashDuration) > 0.0f;
    }

    bool Entity::is_spotted_by_local_player(const driver::Driver& driver, const int& local_player_index) const
    {
        if (local_player_index < 0)
        {
            return false;
        }

        const auto p_entity_spotted_state{entity_pawn_ + client_dll::C_CSPlayerPawn::m_entitySpottedState};
        const auto entity_spotted_by_mask{driver.read<uint64_t>(p_entity_spotted_state + client_dll::EntitySpottedState_t::m_bSpottedByMask)};

        return entity_spotted_by_mask & (static_cast<DWORD64>(1) << (local_player_index));
    }

    util::Vec3 Entity::get_forward_vector(const driver::Driver& driver) const
    {
        const auto movement_services{get_movement_services(driver)};
        return driver.read<util::Vec3>(movement_services + client_dll::CCSPlayer_MovementServices::m_vecForward);
    }

    util::Vec3 Entity::get_vec_origin(const driver::Driver& driver) const
    {
        return driver.read<util::Vec3>(get_game_scene_node(driver) + client_dll::CGameSceneNode::m_vecOrigin);
    }

    util::Vec3 Entity::get_eye_pos(const driver::Driver& driver) const
    {
        return driver.read<util::Vec3>(entity_pawn_ + client_dll::C_BaseModelEntity::m_vecViewOffset) + get_vec_origin(driver);
    }

    util::Vec3 Entity::get_head_bone_pos(const driver::Driver& driver) const
    {
        constexpr auto bone_head{6};

        const auto game_scene_node{get_game_scene_node(driver)};
        const auto bone_array{driver.read<uintptr_t>(game_scene_node + client_dll::CPlayer_MovementServices_Humanoid::m_groundNormal)};
        const auto head{driver.read<util::Vec3>(bone_array + bone_head * 32)};

        return head;
    }

    void Entity::set_spotted(const driver::Driver& driver, const bool& spotted) const
    {
        const auto p_entity_spotted_state{entity_pawn_ + client_dll::C_CSPlayerPawn::m_entitySpottedState};
        const auto entity_spotted_address{p_entity_spotted_state + client_dll::EntitySpottedState_t::m_bSpotted};
        driver.write(entity_spotted_address, spotted);
    }

    void Entity::disable_flash(const driver::Driver& driver) const
    {
        driver.write(entity_pawn_ + client_dll::C_CSPlayerPawnBase::m_flFlashDuration, 0.0f);
    }

    void Entity::set_glowing(const driver::Driver& driver, const bool& glow) const
    {
        driver.write(entity_pawn_ + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_bGlowing, glow);
        if (glow)
        {
            driver.write(entity_pawn_ + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_iGlowType, 3);
            driver.write(entity_pawn_ + client_dll::C_BaseModelEntity::m_Glow + client_dll::CGlowProperty::m_glowColorOverride, IM_COL32(245, 0, 0, 255));
        }
    }
}
