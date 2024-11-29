#include "Cs2CheatController.hpp"

namespace cheat_controller
{
    DWORD Cs2CheatController::getCs2ProcessId()
    {
        const auto processId{commons::process::GetProcessIdByName(g::CS2_PROCESS_NAME)};
        if (!processId.has_value())
        {
            throw std::runtime_error(XOR("Failed to find pid of requested process."));
        }
        std::cout << XOR("Target process id: ") << processId.value() << "\n";

        return processId.value();
    }

    uintptr_t Cs2CheatController::getClientDllBase() const
    {
        const auto clientDllBase{ commons::process::GetModuleBaseAddress(m_cs2ProcessId, g::CLIENT_DLL_MODULE_NAME) };
        if (!clientDllBase.has_value())
        {
            throw std::runtime_error(XOR("Failed GetModuleBaseAddress for target module."));
        }
        std::cout << XOR("Client DLL base: ") << std::hex << std::uppercase << clientDllBase.value() << "\n";
        return clientDllBase.value();
    }

    Cs2CheatController::Cs2CheatController():
        m_driver(driver::Driver{}),
        m_cs2ProcessId(getCs2ProcessId()),
        m_clientDllBase(getClientDllBase())
    {
        if (!m_driver.attach(m_cs2ProcessId))
        {
            throw std::runtime_error(XOR("Failed to attach to process."));
        }
        std::cout << XOR("Attachment successful.\n");
    }
}
