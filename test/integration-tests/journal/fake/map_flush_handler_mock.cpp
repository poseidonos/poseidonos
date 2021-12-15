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
MapFlushHandlerMock::FlushTouchedPages(EventSmartPtr callback)
{
    std::chrono::microseconds sleeptime(5);
    std::this_thread::sleep_for(sleeptime);

    callbackEvent = callback;
    numPagesToFlush = NUM_DIRTY_MPAGES;
    for (int pageId = 0; pageId < NUM_DIRTY_MPAGES; pageId++)
    {
        std::thread flushDone(&MapFlushHandlerMock::_MpageFlushed, this, pageId);
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
