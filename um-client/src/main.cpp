#include <chrono>
#include <Windows.h>
#include <string>
#include <thread>

#include "driver/driver.hpp"
#include "process.hpp"
#include "console.hpp"
#include "controller/Cs2CheatController.hpp"
#include "controller/EntityController.hpp"
#include "sdk/clientDll.hpp"
#include "sdk/sdk.hpp"
#include "sdk/offsets.hpp"

using namespace sdk;
using std::this_thread::sleep_for;
using namespace std::chrono_literals;
using namespace g::settings;
using cheat_controller::entity::EntityController;

int main()
{
    const cheat_controller::Cs2CheatController cheat;

    const auto p_entity_system{cheat.m_client_dll_base + cs2_dumper::offsets::client_dll::dwEntityList};
    std::cout << XOR("Entity System: ") << std::hex << std::uppercase << p_entity_system << '\n';

    sleep_for(2s);

    commons::console::setCursorVisibility(false);
    while (!(GetAsyncKeyState(VK_END) & 0x1))
    {
        commons::console::clearConsole({0, 0});

        std::cout << XOR("[END] Quit\n");
        std::cout << XOR("[F1] Pause\n");
        std::cout << XOR("[F2] Radar hack\n");

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

        EntityController me{cheat.m_driver, cheat.get_local_player_controller(), cheat.get_local_player_pawn()};

        const auto my_team{me.get_team()};

        const auto entity_system{
            cheat.m_driver.read<uintptr_t>(cheat.m_client_dll_base + cs2_dumper::offsets::client_dll::dwEntityList)
        };

        std::cout << '\n';

        for (int i = 1; i < 64; i++)
        {
            const auto entity_controller{cheat.get_entity_controller(entity_system, i)};
            if (!entity_controller.has_value())
            {
                continue;
            }

            const auto entity_pawn{cheat.get_entity_pawn(entity_system, entity_controller.value())};
            if (!entity_pawn.has_value())
            {
                continue;
            }

            const EntityController entity{cheat.m_driver, entity_controller.value(), entity_pawn.value()};

            std::cout << XOR("Name: ") << entity.get_name() << '\n';

            const auto is_local_player{entity.is_local_player()};
            std::cout << XOR("Is this me: ") << (is_local_player ? "yes" : "no") << '\n';

            const auto player_team{entity.get_team()};
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
