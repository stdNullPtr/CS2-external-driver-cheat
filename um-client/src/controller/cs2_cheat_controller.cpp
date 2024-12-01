#include "cs2_cheat_controller.hpp"

namespace cheat
{
    DWORD cs2_cheat_controller::get_cs2_process_id()
    {
        const auto processId{commons::process::GetProcessIdByName(g::CS2_PROCESS_NAME)};
        if (!processId.has_value())
        {
            std::cerr << XOR("Failed to find pid of requested process.\n");
            throw std::runtime_error(XOR("Failed to find pid of requested process."));
        }
        std::cout << XOR("Target process id: ") << processId.value() << '\n';

        return processId.value();
    }

    uintptr_t cs2_cheat_controller::find_client_dll_base() const
    {
        const auto clientDllBase{commons::process::GetModuleBaseAddress(m_cs2_process_id, g::CLIENT_DLL_MODULE_NAME)};
        if (!clientDllBase.has_value())
        {
            std::cerr << XOR("Failed GetModuleBaseAddress for Client DLL.\n");
            throw std::runtime_error(XOR("Failed GetModuleBaseAddress for Client DL."));
        }
        std::cout << XOR("Client DLL base: ") << std::hex << std::uppercase << clientDllBase.value() << '\n';
        return clientDllBase.value();
    }

    uintptr_t cs2_cheat_controller::find_engine_dll_base() const
    {
        const auto clientDllBase{commons::process::GetModuleBaseAddress(m_cs2_process_id, g::ENGINE_DLL_MODULE_NAME)};
        if (!clientDllBase.has_value())
        {
            std::cerr << XOR("Failed GetModuleBaseAddress for Engine DLL.\n");
            throw std::runtime_error(XOR("Failed GetModuleBaseAddress for Engine DLL."));
        }
        std::cout << XOR("Engine DLL base: ") << std::hex << std::uppercase << clientDllBase.value() << '\n';
        return clientDllBase.value();
    }

    cs2_cheat_controller::cs2_cheat_controller(const driver::driver& driver): m_driver(driver)
    {
    }

    DWORD cs2_cheat_controller::get_m_cs2_process_id() const
    {
        return m_cs2_process_id;
    }

    uintptr_t cs2_cheat_controller::get_client_dll_base() const
    {
        return m_client_dll_base;
    }

    uintptr_t cs2_cheat_controller::get_engine_dll_base() const
    {
        return m_engine_dll_base;
    }

    uintptr_t cs2_cheat_controller::get_local_player_controller() const
    {
        return m_driver.read<uintptr_t>(m_client_dll_base + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
    }

    uintptr_t cs2_cheat_controller::get_local_player_pawn() const
    {
        return m_driver.read<uintptr_t>(m_client_dll_base + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    }

    uintptr_t cs2_cheat_controller::get_network_client() const
    {
        return m_driver.read<uintptr_t>(m_engine_dll_base + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient);
    }

    bool cs2_cheat_controller::validate_state_and_re_init()
    {
        if (get_cs2_process_id() != m_cs2_process_id)
        {
            std::cout << XOR("New game instance found, will re-init...\n");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            return init();
        }

        return true;
    }

    bool cs2_cheat_controller::attach() const
    {
        if (!m_driver.attach(m_cs2_process_id))
        {
            std::cerr << XOR("Failed to attach to process.\n");
            return false;
        }
        std::cout << XOR("Attachment successful.\n");
        return true;
    }

    bool cs2_cheat_controller::init()
    {
        auto result{true};

        result &= (m_cs2_process_id = get_cs2_process_id()) != 0;
        result &= (m_client_dll_base = find_client_dll_base()) != 0;
        result &= (m_engine_dll_base = find_engine_dll_base()) != 0;
        result &= attach();

        return result;
    }

    bool cs2_cheat_controller::is_in_game() const
    {
        const auto is_background{m_driver.read<bool>(get_network_client() + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient_isBackgroundMap)};
        const auto state{m_driver.read<int>(get_network_client() + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient_signOnState)};
        return !is_background && state >= 6;
    }

    std::optional<uintptr_t> cs2_cheat_controller::get_entity_controller(const uintptr_t& entity_system, const int& i) const
    {
        const auto listEntity{m_driver.read<uintptr_t>(entity_system + ((8 * (i & 0x7FFF) >> 9) + 16))};
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

    std::optional<uintptr_t> cs2_cheat_controller::get_entity_pawn(const uintptr_t& entity_system, const uintptr_t& entity_controller) const
    {
        const auto entity_controller_pawn{m_driver.read<uintptr_t>(entity_controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn)};
        if (!entity_controller_pawn)
        {
            return std::nullopt;
        }

        const auto list_entity = m_driver.read<uintptr_t>(entity_system + (0x8 * ((entity_controller_pawn & 0x7FFF) >> 9) + 0x10));
        if (!list_entity)
        {
            return std::nullopt;
        }

        const auto entity_pawn{m_driver.read<uintptr_t>(list_entity + 0x78 * (entity_controller_pawn & 0x1FF))};
        if (!entity_pawn)
        {
            std::cerr << XOR("entityPawn is null\n");
            return std::nullopt;
        }

        return entity_pawn;
    }
}
