#pragma once
#include <cstdint>
#include "../util/Vec3.hpp"

namespace sdk
{
    using util::Vec3;

    enum BoneId
    {
        BONE_HEAD = 6,
        BONE_NECK = 5,
        BONE_SPINE = 4,
        BONE_SPINE_1 = 2,
        BONE_HIP = 0,
        BONE_LEFT_SHOULDER = 8,
        BONE_LEFT_ARM = 9,
        BONE_LEFT_HAND = 10,
        BONE_RIGHT_SHOULDER = 13,
        BONE_RIGHT_ARM = 14,
        BONE_RIGHT_HAND = 15,

        BONE_LEFT_HIP = 22,
        BONE_LEFT_KNEE = 23,
        BONE_LEFT_FEET = 24,

        BONE_RIGHT_HIP = 25,
        BONE_RIGHT_KNEE = 26,
        BONE_RIGHT_FEET = 27,
    };

#pragma pack(push, 1)

    struct Bone
    {
        union
        {
            struct
            {
                float _11, _12, _13, _14;
                float _21, _22, _23, _24;
            };

            float m[4][2];
        };
    };

    class CModelState
    {
    public:
        char pad_0000[128]; //0x0000
        Bone (*m_pBoneArray)[32]; //0x0080

        // we can't use a destructor... there is lots of copying starting from driver.read
        void FreeMem() const;
    };

    class CGameSceneNode
    {
    public:
        char pad_0000[136]; //0x0000
        Vec3 m_vecOrigin; //0x0088
        char pad_0094[220]; //0x0094
        CModelState m_modelState; //0x0170
    };

    class C_CSWeaponBase
    {
    public:
        char pad_0000[16]; //0x0000
        class CEntityIdentity* m_pEntity; //0x0010

        // we can't use a destructor... there is lots of copying starting from driver.read
        void FreeMem() const;
    };

    class CEntityIdentity
    {
    public:
        char pad_0000[32]; //0x0000
        char* m_designerName; //0x0020

        // we can't use a destructor... there is lots of copying starting from driver.read
        void FreeMem() const;
    };

    class EntitySpottedState_t
    {
    public:
        char pad_0000[8]; //0x0000
        bool m_bSpotted; //0x0008
        char pad_0009[3]; //0x0009
        uint64_t m_bSpottedByMask; //0x000C
    };

    class C_CSPlayerPawn
    {
    public:
        char pad_0000[808]; //0x0000
        CGameSceneNode* m_pGameSceneNode; //0x0328
        char pad_0330[16]; //0x0330
        int32_t m_iMaxHealth; //0x0340
        int32_t m_iHealth; //0x0344
        char pad_0348[155]; //0x0348
        uint8_t m_iTeamNum; //0x03E3
        char pad_03E4[2157]; //0x03E4
        bool m_bGlowing; //0x0C51
        char pad_0C52[94]; //0x0C52
        Vec3 m_vecViewOffset; //0x0CB0
        char pad_0CBC[1324]; //0x0CBC
        void* m_pMovementServices; //0x11E8
        char pad_11F0[432]; //0x11F0
        C_CSWeaponBase* m_pClippingWeapon; //0x13A0
        char pad_13A8[96]; //0x13A8
        float m_flFlashMaxAlpha; //0x1408
        float m_flFlashDuration; //0x140C
        char pad_1410[4032]; //0x1410
        EntitySpottedState_t m_entitySpottedState; //0x23D0
        char pad_23E4[4]; //0x23E4
        bool m_bIsScoped; //0x23E8
        char pad_23E9[59]; //0x23E9

        // we can't use a destructor... there is lots of copying starting from driver.read
        void FreeMem() const;
    };

#pragma pack(pop)
}
