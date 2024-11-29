#include <chrono>
#include <Windows.h>
#include <string>
#include <cmath>
#include <thread>

#include "driver/driver.hpp"
#include "process.hpp"
#include "console.hpp"
#include "controller/Cs2CheatController.hpp"
#include "sdk/clientDll.hpp"
#include "sdk/sdk.hpp"
#include "sdk/offsets.hpp"

using namespace sdk;
using std::this_thread::sleep_for;
using namespace std::chrono_literals;

int main()
{
    cheat_controller::Cs2CheatController cheat;

    const auto pEntitySystem{cheat.m_clientDllBase + cs2_dumper::offsets::client_dll::dwEntityList};
    std::cout << XOR("Pointer Entity System: ") << std::hex << std::uppercase << pEntitySystem << "\n";

    const auto entitySystem{cheat.m_driver.read<uintptr_t>(pEntitySystem)};
    std::cout << XOR("Entity System: ") << std::hex << std::uppercase << entitySystem << "\n";

    const auto entityList{cheat.m_driver.read<uintptr_t>(entitySystem + 0x10)};
    std::cout << XOR("Entity List: ") << std::hex << std::uppercase << entityList << "\n";

    sleep_for(2s);

    commons::console::setCursorVisibility(false);
    while (!(GetAsyncKeyState(VK_END) & 0x1))
    {
        commons::console::clearConsole({0, 0});

        std::cout << XOR("[END] Quit\n");
        std::cout << XOR("[F1] Pause\n");
        std::cout << XOR("[F2] Radar hack\n");

        static bool isPaused{false};
        static bool radarHack{false};
        if (GetAsyncKeyState(VK_F1) & 0x1)
        {
            isPaused = !isPaused;
        }

        if (isPaused)
        {
            std::cout << XOR("[F1] Paused...\n");
            sleep_for(50ms);
            continue;
        }

        if (GetAsyncKeyState(VK_F2) & 0x1)
        {
            radarHack = !radarHack;
        }

        std::cout << "\n";

        const auto fistEnt{entityList + sizeof Entity /* skip world ent*/};
        std::cout << XOR("fistEnt: ") << std::hex << std::uppercase << fistEnt << "\n";

        const auto localPlayer{cheat.m_driver.read<Entity>(fistEnt)};
        std::cout << XOR("localPlayer.pPlayerController: ") << std::hex << std::uppercase << localPlayer.
            pPlayerController << "\n";

        const auto localPlayerController{
            cheat.m_driver.read<PlayerController>(localPlayer.pPlayerController)
        };
        std::cout << XOR("localPlayerController: ") << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(
            localPlayerController.pad_0000) << "\n";

        std::cout << XOR("[local] m_flSimulationTime: ") << localPlayerController.m_flSimulationTime << "\n";

        std::cout << "\n";

        auto current{fistEnt};
        while (current)
        {
            const auto entity{cheat.m_driver.read<Entity>(current)};
            std::cout << XOR("Entity Base: ") << std::hex << std::uppercase << entity.pPlayerController << "\n";

            const auto playerController{cheat.m_driver.read<PlayerController>(entity.pPlayerController)};
            std::cout << XOR("Player: ") << std::string(playerController.m_iszPlayerName) << "\n";

            const auto pPlayerPawn{entityList + 0x78 * (playerController.m_hPlayerPawn & 0x1FF)};
            std::cout << XOR("pPlayerPawn: ") << std::hex << std::uppercase << pPlayerPawn << "\n";

            const auto playerPawn{cheat.m_driver.read<uintptr_t>(pPlayerPawn)};
            std::cout << XOR("playerPawn: ") << playerPawn << "\n";

            const auto pEntitySpotted{
                playerPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState
            };
            const auto entitySpottedState{cheat.m_driver.read<bool>(pEntitySpotted)};
            std::cout << XOR("entitySpottedState: ") << entitySpottedState << "\n";

            if (radarHack)
            {
                std::cout << XOR("Will radar hack\n");
                if (!entitySpottedState)
                {
                    cheat.m_driver.write(pEntitySpotted, true);
                }
            }

            current = entity.pNext;

            std::cout << "\n";
        }
        sleep_for(100ms);
    }

    return EXIT_SUCCESS;
}
