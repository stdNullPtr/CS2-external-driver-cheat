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
        RenderObjectType type;

        // COMMON
        ImVec4 color;
        ImVec2 position;

        // RECT
        ImVec2 topLeft;
        ImVec2 bottomRight;
        float thickness;

        // TEXT
        std::string text;

    public:
        [[nodiscard]] RenderObjectType get_type() const;

        [[nodiscard]] ImVec2 get_top_left() const;
        [[nodiscard]] ImVec2 get_bottom_right() const;
        [[nodiscard]] ImVec4 get_color() const;
        [[nodiscard]] float get_thickness() const;

        [[nodiscard]] ImVec2 get_position() const;
        [[nodiscard]] std::string get_text() const;

        static DrawCache build_rect(const ImVec2& topLeft, const ImVec2& bottomRight, const bool& filled = false, const ImVec4& col = {255.0f, 0.0f, 0.0f, 255.0f}, const float& thick = 2.0f);

        static DrawCache build_text(const std::string& text, const ImVec2& position, const ImVec4& col);
    };
}
