#pragma once

#include <vector>
#include <mutex>

#include "DrawCache.hpp"

namespace render
{
    class EspDrawList
    {
    public:
        void update(const std::vector<DrawCache>& new_items);
        void clear();

        [[nodiscard]] std::vector<DrawCache> get();

    private:
        std::vector<DrawCache> draw_items_;
        std::mutex mutex_;
    };
}
