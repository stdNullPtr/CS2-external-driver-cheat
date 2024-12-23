#include "C_CSPlayerPawn.hpp"

namespace sdk
{
    void CModelState::FreeMem() const
    {
        delete[] reinterpret_cast<Bone*>(m_pBoneArray);
    }

    void C_CSWeaponBase::FreeMem() const
    {
        delete m_pEntity;
    }

    void CEntityIdentity::FreeMem() const
    {
        delete m_designerName;
    }

    void C_CSPlayerPawn::FreeMem() const
    {
        delete m_pGameSceneNode;
        delete m_pClippingWeapon;
    }
}
