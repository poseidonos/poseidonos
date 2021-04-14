#include "map_flush_handler_mock.h"

#include <thread>
#include <assert.h>

#include "src/event_scheduler/event.h"

namespace pos
{
MapFlushHandlerMock::MapFlushHandlerMock(int mapId)
: id(mapId),
  numPagesToFlush(0),
  numPagesFlushed(0),
  callbackEvent(nullptr)
{
}

MapFlushHandlerMock::~MapFlushHandlerMock(void)
{
}

int
MapFlushHandlerMock::FlushMapWithPageList(MpageList dirtyPages,
    EventSmartPtr callback)
{
    std::chrono::seconds sleeptime(1);
    std::this_thread::sleep_for(sleeptime);

    callbackEvent = callback;
    numPagesToFlush = dirtyPages.size();
    for (auto it = dirtyPages.begin(); it != dirtyPages.end(); ++it)
    {
        std::thread flushDone(&MapFlushHandlerMock::_MpageFlushed, this, *it);
        flushDone.detach();
    }
    return 0;
}

void
MapFlushHandlerMock::_MpageFlushed(int pageId)
{
    std::unique_lock<std::mutex> l(lock);
    numPagesFlushed++;
    if (numPagesToFlush == numPagesFlushed)
    {
        bool executionSuccessful = callbackEvent->Execute();
        assert(executionSuccessful == true);

        numPagesToFlush = 0;
        numPagesFlushed = 0;
    }
}
} // namespace pos
