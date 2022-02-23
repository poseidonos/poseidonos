#pragma once

#include <mutex>
#include <atomic>

#include "src/mapper/include/mpage_info.h"
#include "src/include/smart_ptr_type.h"

namespace pos
{
class MapFlushHandlerMock
{
public:
    explicit MapFlushHandlerMock(int mapId);
    virtual ~MapFlushHandlerMock(void);

    int FlushMapWithPageList(MpageList dirtyPages, EventSmartPtr callback);

private:
    void _MpageFlushed(int pageId);

    int id;
    std::mutex lock;
    std::atomic<int> numPagesToFlush;
    std::atomic<int> numPagesFlushed;
    EventSmartPtr callbackEvent;
};
} // namespace pos
