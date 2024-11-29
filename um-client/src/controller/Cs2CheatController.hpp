#pragma once
#include <process.hpp>

#include "../global.hpp"
#include "../driver/driver.hpp"

namespace cheat_controller
{
    class Cs2CheatController
    {
    public:
        const driver::Driver m_driver;
        const DWORD m_cs2ProcessId;
        const uintptr_t m_clientDllBase;

    private:
        static DWORD getCs2ProcessId();
        uintptr_t getClientDllBase() const;

    public:
        Cs2CheatController();
        ~Cs2CheatController() = default;
    };
}
