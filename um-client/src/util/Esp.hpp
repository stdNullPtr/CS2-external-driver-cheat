#pragma once
#include "Vec2.hpp"

#include "../render/Render.hpp"

namespace util::esp
{
    using render::DrawCache;

    inline DrawCache build_player_esp(const Vec2& entity_eyes_pos_screen, const Vec2& entity_feet_pos_screen)
    {
        constexpr float width_shrink_coefficient{0.35f};
        constexpr float height_shrink_coefficient{0.15f};

        const float width_relative_to_player_distance{(entity_feet_pos_screen.y - entity_eyes_pos_screen.y) * width_shrink_coefficient};
        const float height_relative_to_player_distance{(entity_feet_pos_screen.y - entity_eyes_pos_screen.y) * height_shrink_coefficient};

        const auto main_esp_top_left{ImVec2{entity_eyes_pos_screen.x - width_relative_to_player_distance, entity_eyes_pos_screen.y - height_relative_to_player_distance}};
        const auto main_esp_bot_right{ImVec2{entity_feet_pos_screen.x + width_relative_to_player_distance, entity_feet_pos_screen.y + height_relative_to_player_distance}};

        return DrawCache::build_rect(main_esp_top_left, main_esp_bot_right, false, g::esp_color, g::esp_box_thickness);
    }

    inline std::vector<DrawCache> build_health_esp(const ImVec2& player_esp_top_left, const ImVec2& player_esp_bot_right, const int& health)
    {
        const DrawCache esp_health_border{
            DrawCache::build_rect(
                ImVec2{player_esp_top_left.x - 10.0f, player_esp_top_left.y},
                ImVec2{player_esp_top_left.x, player_esp_bot_right.y},
                false,
                g::esp_color,
                g::esp_box_thickness
            )
        };

        const float health_bar_height{esp_health_border.get_bottom_right().y - esp_health_border.get_top_left().y};
        const float top_left_y{esp_health_border.get_bottom_right().y - (health_bar_height * (static_cast<float>(health) / 100.0f))};
        const DrawCache health_box_filled{
            DrawCache::build_rect(
                ImVec2{
                    esp_health_border.get_top_left().x + esp_health_border.get_thickness(),
                    top_left_y + esp_health_border.get_thickness()
                },
                {
                    esp_health_border.get_bottom_right().x - esp_health_border.get_thickness(),
                    esp_health_border.get_bottom_right().y - esp_health_border.get_thickness()
                },
                true,
                g::esp_health_color,
                g::esp_box_thickness
            )
        };

        const DrawCache health_text{
            DrawCache::build_text(
                std::to_string(health).append("%"),
                {
                    esp_health_border.get_top_left().x + cheat::imgui::g::font_size / 2.0f,
                    esp_health_border.get_top_left().y - cheat::imgui::g::font_size
                },
                g::esp_health_color,
                0
            )
        };

        return {esp_health_border, health_text, health_box_filled};
    }

    inline std::vector<DrawCache> build_player_bottom_esp(const std::string& entity_name, const Vec2& entity_eyes_pos_screen, const ImVec2& player_esp_bot_right, const std::string& weapon_name)
    {
        const DrawCache entity_name_render_obj{
            DrawCache::build_text(
                entity_name,
                ImVec2{entity_eyes_pos_screen.x, player_esp_bot_right.y},
                g::text_color,
                0)
        };

        ImVec4 weapon_text_color{g::text_color};
        if (weapon_name.starts_with(XOR("awp")))
        {
            weapon_text_color = g::weapon_awp_text_color;
        }
        else if (weapon_name.starts_with("knife"))
        {
            weapon_text_color = g::weapon_knife_text_color;
        }
        const DrawCache weapon_name_render_obj{
            DrawCache::build_text(
                weapon_name,
                ImVec2{entity_eyes_pos_screen.x, player_esp_bot_right.y},
                weapon_text_color,
                1)
        };

        return {entity_name_render_obj, weapon_name_render_obj};
    }

    inline std::vector<DrawCache> build_player_top_esp(const bool& is_scoped, const Vec2& entity_eyes_pos_screen, const ImVec2& player_esp_top_left)
    {
        std::vector<DrawCache> result;

        if (is_scoped)
        {
            const auto scoped_text{
                DrawCache::build_text(
                    ICON_FA_EXCLAMATION_TRIANGLE " >SCOPED< " ICON_FA_EXCLAMATION_TRIANGLE,
                    ImVec2{entity_eyes_pos_screen.x, player_esp_top_left.y - cheat::imgui::g::font_size},
                    g::weapon_awp_text_color,
                    0)
            };
            result.emplace_back(scoped_text);
        }

        return result;
    }

    inline std::vector<DrawCache> build_c4_esp(const float& c4_remaining_time, const bool& is_planted_a)
    {
        const auto c4_timer_text{
            DrawCache::build_text(
                std::string{XOR("C4 blow: ")}.append(std::to_string(c4_remaining_time)),
                ImVec2{static_cast<float>(g::additional_screen_info_position_x), static_cast<float>(g::additional_screen_info_position_y)},
                g::additional_screen_info_text_color,
                0)
        };

        const auto c4_bomb_site_text{
            DrawCache::build_text(
                std::string{XOR("Plant site: ")}.append(is_planted_a ? "A" : "B"),
                ImVec2{static_cast<float>(g::additional_screen_info_position_x), static_cast<float>(g::additional_screen_info_position_y)},
                g::additional_screen_info_text_color,
                1)
        };

        return {c4_timer_text, c4_bomb_site_text};
    }
}
