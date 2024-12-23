#pragma once
#include "../sdk/C_CSPlayerPawn.hpp"
#include "../sdk/CCSPlayerController.hpp"

namespace cheat::entity
{
    class Entity
    {
    public:
        sdk::CCSPlayerController controller;
        sdk::C_CSPlayerPawn pawn;

        Entity(const sdk::CCSPlayerController& controllerRef, const sdk::C_CSPlayerPawn& pawnRef)
            : controller(controllerRef), pawn(pawnRef)
        {
        }

        void ReleaseMem() const
        {
            controller.FreeMem();

            if (pawn.m_pGameSceneNode)
            {
                pawn.m_pGameSceneNode->m_modelState.FreeMem();
            }

            if (pawn.m_pClippingWeapon)
            {
                pawn.m_pClippingWeapon->m_pEntity->FreeMem();
                pawn.m_pClippingWeapon->FreeMem();
            }

            pawn.FreeMem();
        }
    };
}
