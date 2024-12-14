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
        ImU32 color;
        float thickness;

        Rect(const ImVec2& topLeft, const ImVec2& bottomRight, const ImU32& col, const float& thick = 1.0f)
            : topLeft(topLeft), bottomRight(bottomRight), color(col), thickness(thick)
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
