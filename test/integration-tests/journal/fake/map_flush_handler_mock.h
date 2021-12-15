#pragma once

#include <atomic>
#include <mutex>

#include "src/include/smart_ptr_type.h"
#include "src/mapper/include/mpage_info.h"

namespace pos
{
class MapFlushHandlerMock
{
public:
    explicit MapFlushHandlerMock(int mapId);
    virtual ~MapFlushHandlerMock(void);

    int FlushTouchedPages(EventSmartPtr callback);

private:
    void _MpageFlushed(int pageId);

    static const int NUM_DIRTY_MPAGES = 10;
    int id;
    std::mutex lock;
    std::atomic<int> numPagesToFlush;
    std::atomic<int> numPagesFlushed;
    EventSmartPtr callbackEvent;
};
} // namespace pos
