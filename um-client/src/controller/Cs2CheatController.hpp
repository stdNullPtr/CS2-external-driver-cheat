#pragma once
#include <chrono>
#include <process.hpp>
#include <optional>

#include "../global.hpp"
#include "../driver/Driver.hpp"
#include "../sdk/dumper/client_dll.hpp"
#include "../sdk/dumper/offsets.hpp"
#include "Entity.hpp"
#include "../util/Vec3.h"
#include "../util/Vec2.h"

namespace cheat
{
    struct viewmatrix_t
    {
        float m[4][4]; // A 4x4 matrix

        // Operator to access elements of the matrix
        const float* operator[](int index) const
        {
            return m[index];
        }

        float* operator[](int index)
        {
            return m[index];
        }

        friend std::wostream& operator<<(std::wostream& os, const viewmatrix_t& matrix)
        {
            os << L"viewmatrix_t:\n";
            for (int i = 0; i < 4; ++i)
            {
                os << L"[ ";
                for (int j = 0; j < 4; ++j)
                {
                    os << std::setw(10) << std::fixed << std::setprecision(3) << matrix[i][j] << L" ";
                }
                os << L"]\n";
            }
            return os;
        }
    };

    class Cs2CheatController
    {
    private:
        DWORD cs2_process_id_{0};
        uintptr_t client_dll_base_{0};
        uintptr_t engine_dll_base_{0};
        uintptr_t entity_system_{0};

    private:
        [[nodiscard]] static std::optional<DWORD> get_cs2_process_id();

        [[nodiscard]] std::optional<uintptr_t> find_client_dll_base() const;
        [[nodiscard]] std::optional<uintptr_t> find_engine_dll_base() const;
        [[nodiscard]] std::optional<uintptr_t> find_entity_system(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t find_local_player_controller(const driver::Driver& driver) const;
        [[nodiscard]] uintptr_t find_local_player_pawn(const driver::Driver& driver) const;

        [[nodiscard]] bool attach(const driver::Driver& driver) const;

    public:
        Cs2CheatController() = default;
        ~Cs2CheatController() = default;

        Cs2CheatController(const Cs2CheatController& other) = delete;
        Cs2CheatController(Cs2CheatController&& other) noexcept = delete;
        Cs2CheatController& operator=(const Cs2CheatController& other) = delete;
        Cs2CheatController& operator=(Cs2CheatController&& other) noexcept = delete;

        [[nodiscard]] static util::Vec2 world_to_screen(const viewmatrix_t& view_matrix, const util::Vec3& world_position);

        [[nodiscard]] uintptr_t get_network_client(const driver::Driver& driver) const;
        [[nodiscard]] bool is_in_game(const driver::Driver& driver) const;
        [[nodiscard]] bool is_state_valid() const;

        [[nodiscard]] std::optional<entity::Entity> get_entity_from_list(const driver::Driver& driver, const int& index) const;
        [[nodiscard]] entity::Entity get_local_player(const driver::Driver& driver) const;
        [[nodiscard]] viewmatrix_t get_view_matrix(const driver::Driver& driver) const;

        [[nodiscard]] bool init(const driver::Driver& driver);

        [[nodiscard]] std::optional<uintptr_t> get_entity_controller(const driver::Driver& driver, const int& i) const;
        [[nodiscard]] std::optional<uintptr_t> get_entity_pawn(const driver::Driver& driver, const uintptr_t& entity_controller) const;
    };
}
