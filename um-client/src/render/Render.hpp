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

    namespace utils
    {
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

        inline bool is_in_screen(const Vec2& location)
        {
            return location.x > 0
                && location.x < static_cast<float>(g::screen_width)
                && location.y > 0
                && location.y < static_cast<float>(g::screen_height);
        }
    }

    inline void set_click_through(const bool& enabled)
    {
        if (enabled)
        {
            SetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE, GetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
        }
        else
        {
            SetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE, GetWindowLong(cheat::imgui::g::hOverlay, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
        }
    }

    inline void draw_scene(const std::stop_token& stop_token, EspDrawList& esp_draw_list)
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
                no_flash_hack = !no_flash_hack;
            }

            if (GetAsyncKeyState(VK_F5) & 0x1)
            {
                esp_hack = !esp_hack;
            }

            if (GetAsyncKeyState(VK_F6) & 0x1)
            {
                aim_hack = !aim_hack;
            }

            if (GetAsyncKeyState(VK_F7) & 0x1 && aim_hack)
            {
                aim_assist = !aim_assist;
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

            ImDrawList* const draw_list{ImGui::GetBackgroundDrawList()};
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

                ImGui::Checkbox(XOR("[F1] Pause "), &is_paused);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_PAUSE);
                ImGui::PopStyleColor();

                if (is_paused)
                {
                    ImGui::BeginDisabled();
                }

                ImGui::Checkbox(XOR("[F2] Radar (Unsafe) "), &radar_hack);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_EXCLAMATION_TRIANGLE);
                ImGui::PopStyleColor();

                ImGui::Checkbox(XOR("[F3] Glow (Unsafe) "), &glow_hack);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_EXCLAMATION_TRIANGLE);
                ImGui::PopStyleColor();

                ImGui::Checkbox(XOR("[F4] No flash (Unsafe)"), &no_flash_hack);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_EXCLAMATION_TRIANGLE);
                ImGui::PopStyleColor();

                ImGui::Checkbox(XOR("[F5] ESP "), &esp_hack);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_EYE);
                ImGui::PopStyleColor();

                ImGui::Checkbox(XOR("[F6] AIM "), &aim_hack);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_CROSSHAIRS);
                ImGui::PopStyleColor();

                if (!aim_hack)
                {
                    ImGui::BeginDisabled();
                }
                ImGui::Checkbox(XOR("[F7] AIM assist (just shoot) "), &aim_assist);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_CROSSHAIRS);
                ImGui::PopStyleColor();
                if (!aim_hack)
                {
                    ImGui::EndDisabled();
                }

                if (is_paused)
                {
                    ImGui::EndDisabled();
                }
                ImGui::PopStyleColor(3);

                ImGui::SeparatorText(XOR("Common"));
                ImGui::Text(XOR("[INS] Minimize "));
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_MINUS);
                ImGui::PopStyleColor();

                ImGui::Text(XOR("[END] Exit "));
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(ICON_FA_TIMES);
                ImGui::PopStyleColor();

                ImGui::Text(XOR("[Open Game console] Free mouse and allow interaction"));

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 128, 255, 255));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
                ImGui::SeparatorText(XOR("ESP Colors"));
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::ColorEdit3(XOR("Box"), reinterpret_cast<float*>(&g::esp_color));
                ImGui::ColorEdit3(XOR("Health"), reinterpret_cast<float*>(&g::esp_health_color));
                ImGui::ColorEdit3(XOR("Text"), reinterpret_cast<float*>(&g::text_color));
                ImGui::ColorEdit3(XOR("Carrying AWP text"), reinterpret_cast<float*>(&g::weapon_awp_text_color));
                ImGui::ColorEdit3(XOR("Carrying knife text"), reinterpret_cast<float*>(&g::weapon_knife_text_color));
                ImGui::ColorEdit3(XOR("Extra info"), reinterpret_cast<float*>(&g::additional_screen_info_text_color));

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 128, 255, 255));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
                ImGui::SeparatorText(XOR("ESP Config"));
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::SliderFloat(XOR("Thickness"), &g::esp_box_thickness, 1.0f, 3.0f);
                ImGui::SliderFloat(XOR("Aim FOV"), &g::aim_fov, 5.0f, 200.0f);
                draw_list->AddCircle(g::screen_center, g::aim_fov, IM_COL32_WHITE, 0, 2.0f);
                ImGui::SliderInt(XOR("Extra info X"), &g::additional_screen_info_position_x, 50, 300);
                ImGui::SliderInt(XOR("Extra info Y"), &g::additional_screen_info_position_y, 0, 700);

                ImGui::End();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
            else
            {
                set_click_through(true);
            }

            const std::vector draw_objects{esp_draw_list.get()};

            for (const auto& object : draw_objects)
            {
                switch (object.get_type())
                {
                case RenderObjectType::rect:
                    draw_list->AddRect(
                        object.get_top_left(),
                        object.get_bottom_right(),
                        ImGui::ColorConvertFloat4ToU32(object.get_color()),
                        0.0f,
                        ImDrawFlags_None,
                        object.get_thickness()
                    );
                    break;
                case RenderObjectType::rect_filled:
                    draw_list->AddRectFilled(
                        object.get_top_left(),
                        object.get_bottom_right(),
                        ImGui::ColorConvertFloat4ToU32(object.get_color()),
                        0.0f,
                        ImDrawFlags_None
                    );
                    break;
                case RenderObjectType::line:
                    break;
                case RenderObjectType::circle:
                    draw_list->AddCircle(object.get_position(), object.get_radius(), IM_COL32_WHITE);
                    break;
                case RenderObjectType::text:
                    const std::string text{object.get_text()};
                    const ImVec4 color{object.get_color()};

                    if (color.x == 0.0f && color.y == 0.0f && color.z == 0.0f)
                    {
                        break;
                    }

                    const ImVec2 requested_position{object.get_position()};

                    const int line_position{object.get_line_position()};
                    const ImVec2 text_size{ImGui::CalcTextSize(text.c_str())};
                    const ImVec2 position{requested_position.x - text_size.x / 2.0f, requested_position.y + text_size.y * static_cast<float>(line_position)};
#pragma region shadow
                    const ImVec4 shadow_color{0.1f, 0.1f, 0.1f, color.w * 0.5f};
                    constexpr float shadow_offset{1.0f};

                    draw_list->AddText(
                        ImVec2(position.x + shadow_offset, position.y),
                        ImGui::ColorConvertFloat4ToU32(shadow_color),
                        text.c_str(),
                        text.c_str() + text.size()
                    );
                    draw_list->AddText(
                        ImVec2(position.x - shadow_offset, position.y),
                        ImGui::ColorConvertFloat4ToU32(shadow_color),
                        text.c_str(),
                        text.c_str() + text.size()
                    );
                    draw_list->AddText(
                        ImVec2(position.x, position.y + shadow_offset),
                        ImGui::ColorConvertFloat4ToU32(shadow_color),
                        text.c_str(),
                        text.c_str() + text.size()
                    );
                    draw_list->AddText(
                        ImVec2(position.x, position.y - shadow_offset),
                        ImGui::ColorConvertFloat4ToU32(shadow_color),
                        text.c_str(),
                        text.c_str() + text.size()
                    );
#pragma endregion
                    draw_list->AddText(
                        position,
                        ImGui::ColorConvertFloat4ToU32(color),
                        text.c_str(),
                        text.c_str() + text.size()
                    );

                    break;
                }
            }

            cheat::imgui::frame::render();
        }

        cheat::imgui::frame::cleanup();
    }
}
