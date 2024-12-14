#pragma once
#include <stop_token>

#include "EspDrawList.hpp"
#include "../global.hpp"
#include "../imgui/ImGuiFrame.hpp"
#include "../util/ViewMatrix.hpp"
#include "../util/Vec2.hpp"
#include "../util/Vec3.hpp"

namespace render
{
    using namespace g::toggles;
    using util::Vec2;
    using util::Vec3;
    using util::ViewMatrix;

    inline Vec2 world_to_screen(const ViewMatrix& view_matrix, const Vec3& world_position)
    {
        float clip_space_x{view_matrix[0][0] * world_position.x + view_matrix[0][1] * world_position.y + view_matrix[0][2] * world_position.z + view_matrix[0][3]};
        float clip_space_y{view_matrix[1][0] * world_position.x + view_matrix[1][1] * world_position.y + view_matrix[1][2] * world_position.z + view_matrix[1][3]};
        const float clip_space_w{view_matrix[3][0] * world_position.x + view_matrix[3][1] * world_position.y + view_matrix[3][2] * world_position.z + view_matrix[3][3]};

        if (clip_space_w <= 0.f)
        {
            return {}; // Return an empty Vec2 if the point is behind the camera
        }

        const float reciprocal_w{1.f / clip_space_w};
        clip_space_x *= reciprocal_w;
        clip_space_y *= reciprocal_w;

        const float screen_x{(1 + clip_space_x) * static_cast<float>(g::screen_width) / 2.0f};
        const float screen_y{(1 - clip_space_y) * static_cast<float>(g::screen_height) / 2.0f};

        return Vec2{.x = screen_x, .y = screen_y};
    }

    inline void set_click_through(const bool& enabled)
    {
        if (enabled)
        {
            // Enable click-through
            SetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE, GetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
        }
        else
        {
            // Disable click-through (allow interaction)
            SetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE, GetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
        }
    }

    inline void draw_scene(const std::stop_token& stop_token, render::EspDrawList& esp_draw_list)
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
                set_click_through(false);
                //TODO this is not always working ????
                //SetForegroundWindow(cheat::imgui::g::hOverlay);

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

                ImGui::ColorEdit3("ESP box color", reinterpret_cast<float*>(&g::espColor));
                ImGui::ColorEdit3("ESP health color", reinterpret_cast<float*>(&g::espHealthColor));

                ImGui::End();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
            else
            {
                set_click_through(true);
            }

            const std::vector rectangles{esp_draw_list.get()};

            //TODO big issue, this is not click-through
            ImDrawList* const draw_list = ImGui::GetBackgroundDrawList();

            for (const auto& rect : rectangles)
            {
                //TODO drop these from here, use top left and top right in the rect object and the hack thread will set them
                if (rect.filled)
                {
                    draw_list->AddRectFilled(rect.topLeft, rect.bottomRight, ImGui::ColorConvertFloat4ToU32(rect.color), 0.0f, ImDrawFlags_None);
                }
                else
                {
                    draw_list->AddRect(rect.topLeft, rect.bottomRight, ImGui::ColorConvertFloat4ToU32(rect.color), 0.0f, ImDrawFlags_None, rect.thickness);
                }
            }

            cheat::imgui::frame::render();
        }

        cheat::imgui::frame::cleanup();
    }
}
