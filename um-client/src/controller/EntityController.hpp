#pragma once
#include <cstdint>
#include "Cs2CheatController.hpp"

namespace cheat_controller::entity
{
    class EntityController
    {
    private:
        const driver::Driver& m_driver;

        const uintptr_t m_entityController;
        const uintptr_t m_entityPawn;

    public:
        EntityController(
            const driver::Driver& driver,
            uintptr_t entityController,
            uintptr_t entityPawn
        );
        ~EntityController() = default;

        EntityController(const EntityController& other) = delete;
        EntityController(EntityController&& other) noexcept = delete;
        EntityController& operator=(const EntityController& other) = delete;
        EntityController& operator=(EntityController&& other) noexcept = delete;

        [[nodiscard]] std::string get_name() const;

        [[nodiscard]] int get_team() const;

        [[nodiscard]] int get_health() const;

        [[nodiscard]] bool is_spotted() const;

        [[nodiscard]] bool is_local_player() const;

        void set_spotted(const bool& new_spotted_state) const;
    };
}
