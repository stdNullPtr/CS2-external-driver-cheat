#include "EspDrawList.hpp"

namespace render
{
    void EspDrawList::update(const std::vector<DrawCache>& new_items)
    {
        std::lock_guard lock(mutex_);
        draw_items_ = new_items;
    }

    void EspDrawList::clear()
    {
        std::lock_guard lock(mutex_);
        draw_items_.clear();
    }

    std::vector<DrawCache> EspDrawList::get()
    {
        std::lock_guard lock(mutex_);
        return draw_items_;
    }
}
