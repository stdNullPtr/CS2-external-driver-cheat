#pragma once
#include "../imgui/lib/imgui.h"
#include <string>

namespace render
{
    enum class RenderObjectType
    {
        rect, rect_filled, line, circle, text
    };

    class DrawCache
    {
    private:
        DrawCache() = default;

    private:
        RenderObjectType type_;

        // COMMON
        ImVec4 color_;
        ImVec2 position_;

        // RECT
        ImVec2 top_left_;
        ImVec2 bottom_right_;
        float thickness_;

        // TEXT
        int line_position_;
        std::string text_;

        //CIRCLE
        float radius_;

    public:
        [[nodiscard]] RenderObjectType get_type() const;

        [[nodiscard]] ImVec4 get_color() const;
        [[nodiscard]] ImVec2 get_position() const;

        [[nodiscard]] ImVec2 get_top_left() const;
        [[nodiscard]] ImVec2 get_bottom_right() const;
        [[nodiscard]] float get_thickness() const;

        [[nodiscard]] int get_line_position() const;
        [[nodiscard]] std::string get_text() const;

        [[nodiscard]] float get_radius() const;

        static DrawCache build_rect(const ImVec2& top_left, const ImVec2& bottom_right, const bool& filled = false, const ImVec4& col = {255.0f, 0.0f, 0.0f, 255.0f}, const float& thick = 2.0f);

        static DrawCache build_text(const std::string& text, const ImVec2& position, const ImVec4& col, const int& line_position);
    };
}
