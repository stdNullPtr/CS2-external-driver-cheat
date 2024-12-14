#include "DrawCache.hpp"

namespace render
{
    RenderObjectType DrawCache::get_type() const
    {
        return type;
    }

    ImVec2 DrawCache::get_top_left() const
    {
        return topLeft;
    }

    ImVec2 DrawCache::get_bottom_right() const
    {
        return bottomRight;
    }

    ImVec4 DrawCache::get_color() const
    {
        return color;
    }

    float DrawCache::get_thickness() const
    {
        return thickness;
    }

    ImVec2 DrawCache::get_position() const
    {
        return position;
    }

    int DrawCache::get_line_position() const
    {
        return line_position;
    }

    std::string DrawCache::get_text() const
    {
        return text;
    }

    DrawCache DrawCache::build_rect(const ImVec2& topLeft, const ImVec2& bottomRight, const bool& filled, const ImVec4& col, const float& thick)
    {
        DrawCache cache;

        cache.type = filled ? RenderObjectType::rect_filled : RenderObjectType::rect;
        cache.topLeft = topLeft;
        cache.bottomRight = bottomRight;
        cache.color = col;
        cache.thickness = thick;

        return cache;
    }

    DrawCache DrawCache::build_text(const std::string& text, const ImVec2& position, const ImVec4& col, const int& line_position)
    {
        DrawCache cache;

        cache.type = RenderObjectType::text;
        cache.position = position;
        cache.color = col;
        cache.text = text;
        cache.line_position = line_position;

        return cache;
    }
}
