#pragma once

#include <vector>
#include <mutex>
#include "../imgui/lib/imgui.h"

namespace draw
{
    struct Rect
    {
        ImVec2 position;
        ImVec2 size;
        ImU32 color;
        float thickness;

        Rect(const ImVec2& pos, const ImVec2& size, const ImU32& col, const float& thick = 2.0f)
            : position(pos), size(size), color(col), thickness(thick)
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
