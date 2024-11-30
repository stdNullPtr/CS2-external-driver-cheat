#pragma once
#include <cstdint>

namespace sdk
{
    class PlayerController
    {
    public:
        char pad_0000[16]; //0x0000
        uintptr_t m_pEntity; //0x0010
        char pad_0018[920]; //0x0018
        float m_flSimulationTime; //0x03B0
        char pad_03B4[684]; //0x03B4
        char m_iszPlayerName[128]; //0x0660
        char pad_06E0[16]; //0x06E0
        bool m_bIsLocalPlayerController; //0x06F0
        char pad_06F1[283]; //0x06F1
        int32_t m_hPlayerPawn; //0x080C
        int32_t m_hObserverPawn; //0x0810
        char pad_0814[28]; //0x0814
    }; //Size: 0x0830

    class Entity
    {
    public:
        uintptr_t pPlayerController; //0x0000
        char pad_0008[80]; //0x0008
        void* pToSomethingThatHasTheWeapon; //0x0058
        char pad_0060[8]; //0x0060
        uintptr_t pNext; //0x0068
        uintptr_t pPrev; //0x0070
    }; //Size: 0x0078
}
