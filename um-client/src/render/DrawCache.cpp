#include "DrawCache.hpp"

namespace render
{
    RenderObjectType DrawCache::get_type() const
    {
        return type_;
    }

    ImVec2 DrawCache::get_top_left() const
    {
        return top_left_;
    }

    ImVec2 DrawCache::get_bottom_right() const
    {
        return bottom_right_;
    }

    ImVec4 DrawCache::get_color() const
    {
        return color_;
    }

    float DrawCache::get_thickness() const
    {
        return thickness_;
    }

    ImVec2 DrawCache::get_position() const
    {
        return position_;
    }

    int DrawCache::get_line_position() const
    {
        return line_position_;
    }

    std::string DrawCache::get_text() const
    {
        return text_;
    }

    float DrawCache::get_radius() const
    {
        return radius_;
    }

    DrawCache DrawCache::build_rect(const ImVec2& top_left, const ImVec2& bottom_right, const bool& filled, const ImVec4& col, const float& thick)
    {
        DrawCache cache;

        cache.type_ = filled ? RenderObjectType::rect_filled : RenderObjectType::rect;
        cache.top_left_ = top_left;
        cache.bottom_right_ = bottom_right;
        cache.color_ = col;
        cache.thickness_ = thick;

        return cache;
    }

    DrawCache DrawCache::build_text(const std::string& text, const ImVec2& position, const ImVec4& col, const int& line_position)
    {
        DrawCache cache;

        cache.type_ = RenderObjectType::text;
        cache.position_ = position;
        cache.color_ = col;
        cache.text_ = text;
        cache.line_position_ = line_position;

        return cache;
    }

    DrawCache DrawCache::build_circle(const ImVec2& position, const ImVec4& col, const float& radius)
    {
        DrawCache cache;

        cache.type_ = RenderObjectType::circle;
        cache.position_ = position;
        cache.color_ = col;
        cache.radius_ = radius;

        return cache;
    }
}
