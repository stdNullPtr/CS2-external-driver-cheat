#include <chrono>
#include <Windows.h>
#include <string>
#include <thread>

#include "driver/Driver.hpp"
#include "process.hpp"
#include "console.hpp"
#include "window.hpp"
#include "controller/Cs2CheatController.hpp"
#include "controller/Entity.hpp"
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

    cheat::Cs2CheatController cheat;

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

            if (!cheat.init(driver))
            {
                std::cerr << XOR("Failed initializing cheat controller state, retying...\n");
                sleep_for(2s);
                continue;
            }
        }

        std::cout << ansi_codes::red << XOR("[END] Quit\n") << ansi_codes::reset_color;
        if (!cheat.is_in_game(driver))
        {
            std::cout << XOR("Waiting for you to join game...\n");
            sleep_for(500ms);
            continue;
        }

        std::cout << '\n';
        std::cout << ansi_codes::light_blue << XOR("[F1] Pause\n") << ansi_codes::reset_color;
        std::cout << ansi_codes::light_blue << XOR("[F2] Radar hack\n") << ansi_codes::reset_color;
        std::cout << ansi_codes::light_blue << XOR("[F3] Glow hack\n") << ansi_codes::reset_color;
        std::cout << '\n';

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

        if (GetAsyncKeyState(VK_F3) & 0x1)
        {
            glow_hack = !glow_hack;
        }

        const auto me{cheat.get_local_player(driver)};

        const auto my_eye_pos{me.get_eye_pos(driver)};
        const auto my_forward_vec{me.get_forward_vector(driver)};
        const auto my_team{me.get_team(driver)};

        std::cout << XOR("Forward vec: ") << my_forward_vec << '\n'
            << XOR("My eye position: ") << my_eye_pos << '\n';

        std::cout << '\n';

        for (int i{1}; i < 32; i++)
        {
            const std::optional entity{cheat.get_entity_from_list(driver, i)};

            if (!entity.has_value())
            {
                continue;
            }

            const auto is_local_player{entity->is_local_player(driver)};
            const auto player_team{entity->get_team(driver)};
            const auto player_health{entity->get_health(driver)};
            const auto entity_spotted{entity->is_spotted(driver)};
            const auto is_glowing{entity->is_glowing(driver)};

            std::cout << XOR("Ent: ") << i << '\n'
                << XOR("Name: ") << entity->get_name(driver) << '\n'
                << XOR("Is this me: ") << (is_local_player ? "yes" : "no") << '\n'
                << XOR("Enemy: ") << (my_team != player_team ? "yes" : "no") << '\n'
                << XOR("HP: ") << std::dec << player_health << '\n'
                << XOR("Visible on Radar: ") << (entity_spotted ? "yes" : "no") << '\n'
                << XOR("Is glowing: ") << (is_glowing ? "yes" : "no") << '\n';

            if (my_team != player_team && player_health > 0 && !is_local_player)
            {
                if (glow_hack)
                {
                    entity->set_glowing(driver, true);
                }

                if (radar_hack && !entity_spotted)
                {
                    //TODO should we set the correct mask as well? we are setting this bool but the variables after it should be 1 as well
                    entity->set_spotted(driver, true);
                }
            }
            std::cout << '\n';
        }

        sleep_for(20ms);
    }

    return EXIT_SUCCESS;
}
