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
#include "imgui/ImGuiFrame.h"

using namespace std::chrono_literals;
using namespace g::toggles;
using namespace commons::console;
using std::this_thread::sleep_for;

int main()
{
#pragma region imgui_loop

    /*// Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    cheat::imgui::init();

    // Main loop
    bool done = false;
    while (!done)
    {
        const auto frameState{ cheat::imgui::frame::startFrame()};
        if (frameState == cheat::imgui::frame::FRAME_QUIT)
        {
            break;
        }

        if (frameState == cheat::imgui::frame::FRAME_SKIP)
        {
            continue;
        }

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        cheat::imgui::frame::render(clear_color);
    }

    cheat::imgui::frame::cleanup();

    return 1;*/
#pragma endregion imgui_loop

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