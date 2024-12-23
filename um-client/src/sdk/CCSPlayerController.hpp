#pragma once
#include <cstdint>

namespace sdk
{
#pragma pack(push, 1)
	class CCSPlayerController
	{
	public:
		char pad_0000[1776]; //0x0000
		bool m_bIsLocalPlayerController; //0x06F0
		char pad_06F1[127]; //0x06F1
		char* m_sSanitizedPlayerName; //0x0770
		char pad_0778[148]; //0x0778
		uint64_t m_hPlayerPawn; //0x080C

		// we can't use a destructor... there is lots of copying starting from driver.read
		void FreeMem() const;
    };
#pragma pack(pop)
}
