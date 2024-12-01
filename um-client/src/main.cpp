#include <chrono>
#include <Windows.h>
#include <string>
#include <thread>

#include "driver/Driver.hpp"
#include "process.hpp"
#include "console.hpp"
#include "window.hpp"
#include "controller/Cs2CheatController.hpp"
#include "controller/EntityController.hpp"
#include "sdk/dumper/client_dll.hpp"
#include "sdk/dumper/offsets.hpp"

using namespace std::chrono_literals;
using namespace g::toggles;
using namespace commons::console;
using std::this_thread::sleep_for;

int main()
{
    setCursorVisibility(false);

    const driver::Driver driver{};
    if (!driver.is_valid())
    {
        MessageBox(nullptr, XOR("Load the Driver first lmao."), XOR("Oopsie"), MB_OK);
        return EXIT_FAILURE;
    }

    cheat::Cs2CheatController cheat{driver};

    sleep_for(2s);

    while (!(GetAsyncKeyState(VK_END) & 0x1))
    {
        clearConsole({0, 0});

        if (!commons::window::waitForWindow(XOR("Counter-Strike 2"), 999999h))
        {
            std::cerr << XOR("Aborted looking for game window, exiting...\n");
            break;
        }

        if (!cheat.is_state_valid())
        {
            std::cout << XOR("Controller state invalid, will re-init...\n");
            reset_toggles();
            sleep_for(3s);

            if (!cheat.init())
            {
                std::cerr << XOR("Failed initializing cheat controller state, retying...\n");
                sleep_for(2s);
                continue;
            }
        }

        std::cout << ansi_codes::red << XOR("[END] Quit\n") << ansi_codes::reset_color;
        if (!cheat.is_in_game())
        {
            std::cout << XOR("Waiting for you to join game...\n");
            sleep_for(500ms);
            continue;
        }

        std::cout << ansi_codes::light_blue << XOR("[F1] Pause\n") << ansi_codes::reset_color;
        std::cout << ansi_codes::light_blue << XOR("[F2] Radar hack\n") << ansi_codes::reset_color;

        if (GetAsyncKeyState(VK_F1) & 0x1)
        {
            is_paused = !is_paused;
        }

        if (is_paused)
        {
            std::cout << XOR("[F1] Paused...\n");
            sleep_for(50ms);
            continue;
        }

        if (GetAsyncKeyState(VK_F2) & 0x1)
        {
            radar_hack = !radar_hack;
        }

        cheat::entity::EntityController me{cheat.m_driver, cheat.get_local_player_controller(), cheat.get_local_player_pawn()};

        const auto entity_system{cheat.m_driver.read<uintptr_t>(cheat.get_client_dll_base() + cs2_dumper::offsets::client_dll::dwEntityList)};

        std::cout << '\n';

        for (int i = 1; i < 32; i++)
        {
            const auto controller{cheat.get_entity_controller(entity_system, i)};
            if (!controller.has_value())
            {
                continue;
            }

            const auto entity_pawn{cheat.get_entity_pawn(entity_system, controller.value())};
            if (!entity_pawn.has_value())
            {
                continue;
            }

            const cheat::entity::EntityController entity{cheat.m_driver, controller.value(), entity_pawn.value()};

            std::cout << XOR("Name: ") << entity.get_name() << '\n';

            const auto is_local_player{entity.is_local_player()};
            std::cout << XOR("Is this me: ") << (is_local_player ? "yes" : "no") << '\n';

            const auto player_team{entity.get_team()};
            const auto my_team{me.get_team()};
            std::cout << XOR("Enemy: ") << (my_team != player_team ? "yes" : "no") << '\n';

            const auto player_health{entity.get_health()};
            std::cout << XOR("HP: ") << std::dec << player_health << '\n';

            const auto entity_spotted{entity.is_spotted()};
            std::cout << XOR("Visible on Radar: ") << (entity_spotted ? "yes" : "no") << '\n';

            if (radar_hack && my_team != player_team && player_health > 0 && !is_local_player)
            {
                std::cout << XOR("Will radar hack\n");
                if (!entity_spotted)
                {
                    //TODO should we set the correct mask as well? we are setting this bool but the variables after it should be 1 as well
                    entity.set_spotted(true);
                }
            }
            std::cout << '\n';
        }

        sleep_for(100ms);
    }

    return EXIT_SUCCESS;
}
