#pragma once
#include <cstdint>
#include "cs2_cheat_controller.hpp"

namespace cheat_controller::entity
{
    class entity_controller
    {
    private:
        const driver::driver& m_driver;

        const uintptr_t m_entityController;
        const uintptr_t m_entityPawn;

    public:
        entity_controller(const driver::driver& driver, uintptr_t entity_controller, uintptr_t entity_pawn);
        ~entity_controller() = default;

        entity_controller(const entity_controller& other) = delete;
        entity_controller(entity_controller&& other) noexcept = delete;
        entity_controller& operator=(const entity_controller& other) = delete;
        entity_controller& operator=(entity_controller&& other) noexcept = delete;

        [[nodiscard]] std::string get_name() const;

        [[nodiscard]] int get_team() const;

        [[nodiscard]] int get_health() const;

        [[nodiscard]] bool is_spotted() const;

        [[nodiscard]] bool is_local_player() const;

        void set_spotted(const bool& new_spotted_state) const;
    };
}
