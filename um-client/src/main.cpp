#include <Windows.h>
#include <string>
#include <cmath>
#include "driver/driver.hpp"
#include "process.hpp"
#include "console.hpp"
#include "global.hpp"
#include "sdk/clientDll.hpp"
#include "sdk/sdk.hpp"
#include "sdk/offsets.hpp"

using namespace sdk;

int main()
{
    const auto processId{commons::process::GetProcessIdByName(g::CS2_PROCESS_NAME)};
    if (!processId.has_value())
    {
        std::cout << XOR("Failed to find pid of requested process\n");
        return EXIT_FAILURE;
    }
    std::cout << XOR("Process id: ") << processId.value() << "\n";

    const HANDLE driverHandle = CreateFile(
        R"(\\.\xd)",
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (driverHandle == INVALID_HANDLE_VALUE)
    {
        std::cout << XOR("Failed to create our driver handle. Error: ") << GetLastError() << "\n";
        return EXIT_FAILURE;
    }
    std::cout << XOR("Created driver handle.\n");

    const driver::Driver driver;

    if (!driver.attach(processId.value()))
    {
        std::cout << XOR("Failed to attach to process.\n");
        return EXIT_FAILURE;
    }
    std::cout << XOR("Attachment successful.\n");

    const auto clientDllBase{commons::process::GetModuleBaseAddress(processId.value(), g::CLIENT_DLL_MODULE_NAME)};
    if (!clientDllBase.has_value())
    {
        std::cout << XOR("Failed GetModuleBaseAddress for target module\n");
        return EXIT_FAILURE;
    }
    std::cout << XOR("Client DLL base: ") << std::hex << std::uppercase << clientDllBase.value() << "\n";

    const auto pEntitySystem{clientDllBase.value() + cs2_dumper::offsets::client_dll::dwEntityList};
    std::cout << XOR("Pointer Entity System: ") << std::hex << std::uppercase << pEntitySystem << "\n";

    const auto entitySystem{driver.read<uintptr_t>(pEntitySystem)};
    std::cout << XOR("Entity System: ") << std::hex << std::uppercase << entitySystem << "\n";

    const auto entityList{driver.read<uintptr_t>(entitySystem + 0x10)};
    std::cout << XOR("Entity List: ") << std::hex << std::uppercase << entityList << "\n";

    Sleep(2000);

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
            Sleep(50);
            continue;
        }

        if (GetAsyncKeyState(VK_F2) & 0x1)
        {
            radarHack = !radarHack;
        }

        std::cout << "\n";

        const auto fistEnt{entityList + sizeof Entity /* skip world ent*/};
        std::cout << XOR("fistEnt: ") << std::hex << std::uppercase << fistEnt << "\n";

        const auto localPlayer{driver.read<Entity>(fistEnt)};
        std::cout << XOR("localPlayer.pPlayerController: ") << std::hex << std::uppercase << localPlayer.
            pPlayerController << "\n";

        const auto localPlayerController{
            driver.read<PlayerController>(localPlayer.pPlayerController)
        };
        std::cout << XOR("localPlayerController: ") << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(
            localPlayerController.pad_0000) << "\n";

        std::cout << XOR("[local] m_flSimulationTime: ") << localPlayerController.m_flSimulationTime << "\n";

        std::cout << "\n";

        const auto dwEntityList{
            driver.read<uintptr_t>(clientDllBase.value() + cs2_dumper::offsets::client_dll::dwEntityList)
        };
        for (int i = 1; i < 64; ++i)
        {
            const auto controllerEntry{
                driver.read<uintptr_t>(dwEntityList + (0x8 * (i & 0x7FFF) >> 9) + 0x10)
            };
            if (!controllerEntry)
            {
                continue;
            }

            const auto controller{driver.read<uintptr_t>(controllerEntry + 0x78 * (i & 0x1FF))};
            if (!controller)
            {
                continue;
            }

            const auto pawnEntry{controller + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn};
            if (!pawnEntry)
            {
                continue;
            }

            const auto listEntry2{
                driver.read<uintptr_t>(entityList + 0x8 * ((pawnEntry & 0x7FFF) >> 9) + 16)
            };
            if (!listEntry2)
            {
                continue;
            }

            const auto playerPawn{driver.read<uintptr_t>(listEntry2 + 120 * (pawnEntry & 0x1FF))};
            if (!playerPawn)
            {
                continue;
            }
        }

        auto current{fistEnt};
        while (current)
        {
            const auto entity{
                driver.read<Entity>(current)
            };

            std::cout << XOR("Entity Base: ") << std::hex << std::uppercase << entity.pPlayerController << "\n";

            const auto playerController{driver.read<PlayerController>(entity.pPlayerController)};

            std::cout << XOR("Player: ") << std::string(playerController.m_iszPlayerName) << "\n";

            const auto pPlayerPawn{entityList + 0x78 * (playerController.m_hPlayerPawn & 0x1FF)};
            std::cout << XOR("pPlayerPawn: ") << std::hex << std::uppercase << pPlayerPawn << "\n";
            const auto playerPawn{driver.read<uintptr_t>(pPlayerPawn)};
            std::cout << XOR("playerPawn: ") << playerPawn << "\n";

            const auto pEntitySpotted{
                playerPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState
            };
            const auto entitySpottedState{driver.read<bool>(pEntitySpotted)};
            std::cout << XOR("entitySpottedState: ") << entitySpottedState << "\n";

            if (radarHack)
            {
                std::cout << XOR("Will radar hack\n");
                if (!entitySpottedState)
                {
                    driver.write(pEntitySpotted, true);
                }
            }

            current = entity.pNext;

            std::cout << "\n";
        }

        Sleep(100);
    }

    CloseHandle(driverHandle);

    return EXIT_SUCCESS;
}
