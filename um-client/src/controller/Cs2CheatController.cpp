#include "Cs2CheatController.hpp"

namespace cheat_controller
{
    DWORD Cs2CheatController::getCs2ProcessId()
    {
        const auto processId{commons::process::GetProcessIdByName(g::CS2_PROCESS_NAME)};
        if (!processId.has_value())
        {
            std::cerr << "Failed to find pid of requested process.\n";
            throw std::runtime_error(XOR("Failed to find pid of requested process."));
        }
        std::cout << XOR("Target process id: ") << processId.value() << '\n';

        return processId.value();
    }

    uintptr_t Cs2CheatController::getClientDllBase() const
    {
        const auto clientDllBase{commons::process::GetModuleBaseAddress(m_cs2_process_id, g::CLIENT_DLL_MODULE_NAME)};
        if (!clientDllBase.has_value())
        {
            std::cerr << "Failed GetModuleBaseAddress for target module.\n";
            throw std::runtime_error(XOR("Failed GetModuleBaseAddress for target module."));
        }
        std::cout << XOR("Client DLL base: ") << std::hex << std::uppercase << clientDllBase.value() << '\n';
        return clientDllBase.value();
    }

    Cs2CheatController::Cs2CheatController():
        m_driver(driver::Driver{}),
        m_cs2_process_id(getCs2ProcessId()),
        m_client_dll_base(getClientDllBase())
    {
        if (!m_driver.attach(m_cs2_process_id))
        {
            throw std::runtime_error(XOR("Failed to attach to process."));
        }
        std::cout << XOR("Attachment successful.\n");
    }

    uintptr_t Cs2CheatController::get_local_player_controller() const
    {
        return m_driver.read<uintptr_t>(m_client_dll_base + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
    }

    uintptr_t Cs2CheatController::get_local_player_pawn() const
    {
        return m_driver.read<uintptr_t>(m_client_dll_base + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    }

    std::optional<uintptr_t> Cs2CheatController::get_entity_controller(const uintptr_t& entity_system,
                                                                       const int& i) const
    {
        const auto listEntity{
            m_driver.read<uintptr_t>(entity_system + ((8 * (i & 0x7FFF) >> 9) + 16))
        };
        if (!listEntity)
        {
            return std::nullopt;
        }

        const auto entityController{m_driver.read<uintptr_t>(listEntity + 0x78 * (i & 0x1FF))};
        if (!entityController)
        {
            return std::nullopt;
        }

        return entityController;
    }

    std::optional<uintptr_t> Cs2CheatController::get_entity_pawn(const uintptr_t& entity_system,
                                                                 const uintptr_t& entity_controller) const
    {
        const auto entityControllerPawn{
            m_driver.read<uintptr_t>(
                entity_controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn)
        };
        if (!entityControllerPawn)
        {
            return std::nullopt;
        }

        const auto listEntity = m_driver.read<uintptr_t>(
            entity_system + (0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 0x10));
        if (!listEntity)
        {
            return std::nullopt;
        }

        const auto entityPawn{m_driver.read<uintptr_t>(listEntity + 0x78 * (entityControllerPawn & 0x1FF))};
        if (!entityPawn)
        {
            std::cerr << XOR("entityPawn is null\n");
            return std::nullopt;
        }

        return entityPawn;
    }
}
