#include "Cs2CheatController.hpp"

namespace cheat
{
    std::optional<DWORD> Cs2CheatController::get_cs2_process_id()
    {
        const auto process_id{commons::process::GetProcessIdByName(g::cs2_process_name)};
        if (!process_id.has_value())
        {
            std::wcerr << XORW(L"Failed to find pid of requested process.\n");
            return std::nullopt;
        }

#ifndef NDEBUG
        std::wcout << XORW(L"Target process id: ") << process_id.value() << '\n';
#endif

        return process_id;
    }

    std::optional<uintptr_t> Cs2CheatController::find_client_dll_base() const
    {
        const auto client_dll_base{commons::process::GetModuleBaseAddress(cs2_process_id_, g::client_dll_module_name)};
        if (!client_dll_base.has_value())
        {
            std::wcerr << XORW(L"Failed GetModuleBaseAddress for Client DLL.\n");
            return std::nullopt;
        }

#ifndef NDEBUG
        std::wcout << XORW(L"Client DLL base: ") << std::hex << std::uppercase << client_dll_base.value() << '\n';
#endif

        return client_dll_base;
    }

    std::optional<uintptr_t> Cs2CheatController::find_engine_dll_base() const
    {
        const auto engine_dll_base{commons::process::GetModuleBaseAddress(cs2_process_id_, g::engine_dll_module_name)};
        if (!engine_dll_base.has_value())
        {
            std::wcerr << XORW(L"Failed GetModuleBaseAddress for Engine DLL.\n");
            return std::nullopt;
        }

#ifndef NDEBUG
        std::wcout << XORW(L"Engine DLL base: ") << std::hex << std::uppercase << engine_dll_base.value() << '\n';
#endif

        return engine_dll_base;
    }

    std::optional<uintptr_t> Cs2CheatController::find_entity_system(const driver::Driver& driver) const
    {
        if (!client_dll_base_)
        {
            return std::nullopt;
        }

        return driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwEntityList);
    }

    uintptr_t Cs2CheatController::find_local_player_controller(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
    }

    uintptr_t Cs2CheatController::find_local_player_pawn(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    }

    uintptr_t Cs2CheatController::get_network_client(const driver::Driver& driver) const
    {
        return driver.read<uintptr_t>(engine_dll_base_ + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient);
    }

    bool Cs2CheatController::attach(const driver::Driver& driver) const
    {
        if (!driver.attach(cs2_process_id_))
        {
            std::wcerr << XORW(L"Failed to attach to process.\n");
            return false;
        }
        std::wcout << XORW(L"Attachment successful.\n");
        return true;
    }

    bool Cs2CheatController::init(const driver::Driver& driver)
    {
        auto result{true};

        result &= (cs2_process_id_ = get_cs2_process_id().value_or(0)) != 0;
        result &= (client_dll_base_ = find_client_dll_base().value_or(0)) != 0;
        result &= (engine_dll_base_ = find_engine_dll_base().value_or(0)) != 0;
        result &= (entity_system_ = find_entity_system(driver).value_or(0)) != 0;
        result &= attach(driver);

        return result;
    }

    bool Cs2CheatController::build_number_changed(const driver::Driver& driver) const
    {
        //TODO read the json info.json dont be an idiot
        static constexpr DWORD expected_build{14055};

        return driver.read<DWORD>(engine_dll_base_ + cs2_dumper::offsets::engine2_dll::dwBuildNumber) != expected_build;
    }

    bool Cs2CheatController::is_in_game(const driver::Driver& driver) const
    {
        const auto is_background{driver.read<bool>(get_network_client(driver) + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient_isBackgroundMap)};
        const auto state{driver.read<int>(get_network_client(driver) + cs2_dumper::offsets::engine2_dll::dwNetworkGameClient_signOnState)};
        return !is_background && state >= 6;
    }

    bool Cs2CheatController::is_state_valid() const
    {
        return get_cs2_process_id() == cs2_process_id_;
    }

    std::optional<entity::Entity> Cs2CheatController::get_entity_from_list(const driver::Driver& driver, const int& index) const
    {
        const auto controller{get_entity_controller(driver, index)};
        if (!controller)
        {
            return std::nullopt;
        }

        const auto entity_pawn{get_entity_pawn(driver, *controller)};
        if (!entity_pawn)
        {
            return std::nullopt;
        }

        return entity::Entity{*controller, *entity_pawn};
    }

    entity::Entity Cs2CheatController::get_local_player(const driver::Driver& driver) const
    {
        return entity::Entity{find_local_player_controller(driver), find_local_player_pawn(driver)};
    }

    util::ViewMatrix Cs2CheatController::get_view_matrix(const driver::Driver& driver) const
    {
        return driver.read<util::ViewMatrix>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwViewMatrix);
    }

    float Cs2CheatController::c4_blow_remaining_time(const driver::Driver& driver) const
    {
        const auto dw_game_rules{driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwGameRules)};
        const auto bomb_planted{driver.read<bool>(dw_game_rules + cs2_dumper::schemas::client_dll::C_CSGameRules::m_bBombPlanted)};

        if (!bomb_planted)
        {
            return 0.0f;
        }

        const auto planted_c4{driver.read<uintptr_t>(driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwPlantedC4))};
        const auto global_vars{driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwGlobalVars)};

        const auto c4_time{driver.read<float>(planted_c4 + cs2_dumper::schemas::client_dll::C_PlantedC4::m_flC4Blow)};
        const auto global_time{driver.read<float>(global_vars + 0x34)};

        return c4_time - global_time;
    }

    bool Cs2CheatController::c4_is_bomb_site_a(const driver::Driver& driver) const
    {
        const auto planted_c4{driver.read<uintptr_t>(driver.read<uintptr_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwPlantedC4))};
        const auto bomb_site{driver.read<int>(planted_c4 + cs2_dumper::schemas::client_dll::C_PlantedC4::m_nBombSite)};

        return bomb_site == 0;
    }

    std::optional<uintptr_t> Cs2CheatController::get_entity_controller(const driver::Driver& driver, const int& i) const
    {
        const auto list_entity{driver.read<uintptr_t>(entity_system_ + ((8 * (i & 0x7FFF) >> 9) + 16))};
        if (!list_entity)
        {
            return std::nullopt;
        }

        const auto entity_controller{driver.read<uintptr_t>(list_entity + 0x78 * (i & 0x1FF))};
        if (!entity_controller)
        {
            return std::nullopt;
        }

        return entity_controller;
    }

    std::optional<uintptr_t> Cs2CheatController::get_entity_pawn(const driver::Driver& driver, const uintptr_t& entity_controller) const
    {
        const auto entity_controller_pawn{driver.read<uintptr_t>(entity_controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn)};
        if (!entity_controller_pawn)
        {
            return std::nullopt;
        }

        const auto list_entity = driver.read<uintptr_t>(entity_system_ + (0x8 * ((entity_controller_pawn & 0x7FFF) >> 9) + 0x10));
        if (!list_entity)
        {
            return std::nullopt;
        }

        const auto entity_pawn{driver.read<uintptr_t>(list_entity + 0x78 * (entity_controller_pawn & 0x1FF))};
        if (!entity_pawn)
        {
            std::wcerr << XORW(L"Cant grab player pawn, maybe the player disconnected?\n");
            return std::nullopt;
        }

        return entity_pawn;
    }
}
