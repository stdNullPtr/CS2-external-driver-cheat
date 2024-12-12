#pragma once
#include <stop_token>

#include "EspDrawList.hpp"
#include "../global.hpp"
#include "../imgui/ImGuiFrame.h"

namespace draw
{
    using namespace g::toggles;

    void draw_imgui(const std::stop_token& stop_token, EspDrawList& esp_draw_list)
    {
        cheat::imgui::init();

        while (!stop_token.stop_requested())
        {
            if (GetAsyncKeyState(VK_F1) & 0x1)
            {
                is_paused = !is_paused;
            }

            if (GetAsyncKeyState(VK_F2) & 0x1)
            {
                radar_hack = !radar_hack;
            }

            if (GetAsyncKeyState(VK_F3) & 0x1)
            {
                glow_hack = !glow_hack;
            }

            if (GetAsyncKeyState(VK_F4) & 0x1)
            {
                esp_hack = !esp_hack;
            }

            if (GetAsyncKeyState(VK_INSERT) & 0x1)
            {
                show_menu = !show_menu;
            }

            const auto frame_state{cheat::imgui::frame::startFrame()};

            if (frame_state == cheat::imgui::frame::FRAME_QUIT)
            {
                break;
            }

            if (frame_state == cheat::imgui::frame::FRAME_SKIP)
            {
                continue;
            }

            if (show_menu)
            {
                ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 230));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

                ImGui::Begin(XOR("XD"), &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 128, 255, 255));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
                ImGui::SeparatorText(XOR("Toggles"));
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(50, 50, 50, 255));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(70, 70, 70, 255));
                ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(0, 128, 255, 255));
                ImGui::Checkbox(XOR("[F1] Pause"), &is_paused);
                if (is_paused)
                {
                    ImGui::BeginDisabled();
                }
                ImGui::Checkbox(XOR("[F2] Radar (Unsafe)"), &radar_hack);
                ImGui::Checkbox(XOR("[F3] Glow (Unsafe)"), &glow_hack);
                ImGui::Checkbox(XOR("[F4] ESP"), &esp_hack);
                if (is_paused)
                {
                    ImGui::EndDisabled();
                }
                ImGui::PopStyleColor(3);

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 0, 0, 255));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
                ImGui::SeparatorText(XOR("[INS] Hide"));
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::End();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }

            const std::vector rectangles{esp_draw_list.get()};

            //TODO big issue, this is not click-through
            ImDrawList* const draw_list = ImGui::GetBackgroundDrawList();

            for (const auto& rect : rectangles)
            {
                //TODO drop these from here, use top left and top right in the rect object and the hack thread will set them
                const auto top_left{ImVec2(rect.position.x - rect.size.x * 0.5f, rect.position.y - rect.size.y * 0.5f)};
                const auto bottom_right{ImVec2(rect.position.x + rect.size.x * 0.5f, rect.position.y + rect.size.y * 0.5f)};
                //TODO maybe instead of addrect these actually have to be windows? It makes more sense since we can highly customize them instead of having simple lines
                draw_list->AddRect(top_left, bottom_right, rect.color, 0.0f, ImDrawFlags_None, rect.thickness);
            }

            cheat::imgui::frame::render();
        }

        cheat::imgui::frame::cleanup();
    }
}
