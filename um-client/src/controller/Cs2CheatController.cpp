#include "Cs2CheatController.hpp"

namespace cheat
{
    std::optional<DWORD> Cs2CheatController::get_cs2_process_id()
    {
        const auto process_id{commons::process::GetProcessIdByName(g::CS2_PROCESS_NAME)};
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
        const auto client_dll_base{commons::process::GetModuleBaseAddress(cs2_process_id_, g::CLIENT_DLL_MODULE_NAME)};
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
        const auto engine_dll_base{commons::process::GetModuleBaseAddress(cs2_process_id_, g::ENGINE_DLL_MODULE_NAME)};
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

    util::Vec2 Cs2CheatController::world_to_screen(const viewmatrix_t& view_matrix, const util::Vec3& world_position)
    {
        static const int screen_width{GetSystemMetrics(SM_CXSCREEN)};
        static const int screen_height{GetSystemMetrics(SM_CYSCREEN)};

        float clip_space_x{view_matrix[0][0] * world_position.x + view_matrix[0][1] * world_position.y + view_matrix[0][2] * world_position.z + view_matrix[0][3]};
        float clip_space_y{view_matrix[1][0] * world_position.x + view_matrix[1][1] * world_position.y + view_matrix[1][2] * world_position.z + view_matrix[1][3]};
        const float clip_space_w{view_matrix[3][0] * world_position.x + view_matrix[3][1] * world_position.y + view_matrix[3][2] * world_position.z + view_matrix[3][3]};

        if (clip_space_w <= 0.f)
        {
            return {}; // Return an empty Vec2 if the point is behind the camera
        }

        const float reciprocal_w{1.f / clip_space_w};
        clip_space_x *= reciprocal_w;
        clip_space_y *= reciprocal_w;

        const float screen_x{(1 + clip_space_x) * static_cast<float>(screen_width) / 2.0f};
        const float screen_y{(1 - clip_space_y) * static_cast<float>(screen_height) / 2.0f};

        return util::Vec2{.x = screen_x, .y = screen_y};
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
        if (!controller.has_value())
        {
            return std::nullopt;
        }

        const auto entity_pawn{get_entity_pawn(driver, controller.value())};
        if (!entity_pawn.has_value())
        {
            return std::nullopt;
        }

        return entity::Entity{controller.value(), entity_pawn.value()};
    }

    entity::Entity Cs2CheatController::get_local_player(const driver::Driver& driver) const
    {
        return entity::Entity{find_local_player_controller(driver), find_local_player_pawn(driver)};
    }

    viewmatrix_t Cs2CheatController::get_view_matrix(const driver::Driver& driver) const
    {
        return driver.read<viewmatrix_t>(client_dll_base_ + cs2_dumper::offsets::client_dll::dwViewMatrix);
    }

    std::optional<uintptr_t> Cs2CheatController::get_entity_controller(const driver::Driver& driver, const int& i) const
    {
        const auto listEntity{driver.read<uintptr_t>(entity_system_ + ((8 * (i & 0x7FFF) >> 9) + 16))};
        if (!listEntity)
        {
            return std::nullopt;
        }

        const auto entityController{driver.read<uintptr_t>(listEntity + 0x78 * (i & 0x1FF))};
        if (!entityController)
        {
            return std::nullopt;
        }

        return entityController;
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
            std::wcerr << XORW(L"entityPawn is null\n");
            return std::nullopt;
        }

        return entity_pawn;
    }
}
