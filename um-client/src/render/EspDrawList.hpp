#pragma once

#include <vector>
#include <mutex>
#include "../imgui/lib/imgui.h"

namespace render
{
    struct Rect
    {
        ImVec2 topLeft;
        ImVec2 bottomRight;
        ImVec4 color;
        bool filled;
        float thickness;

        Rect(const ImVec2& topLeft, const ImVec2& bottomRight, const ImVec4& col = {255.0f, 0.0f, 0.0f, 255.0f}, const bool& filled = false, const float& thick = 2.0f)
            : topLeft(topLeft), bottomRight(bottomRight), color(col), filled(filled), thickness(thick)
        {
        }
    };

    class EspDrawList
    {
    public:
        void update(const std::vector<Rect>& new_items);
        void clear();

        [[nodiscard]] std::vector<Rect> get();

    private:
        std::vector<Rect> draw_items_;
        std::mutex mutex_;
    };
}
