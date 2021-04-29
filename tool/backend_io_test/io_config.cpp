#define UNVME_BUILD

#include "io_config.h"

#include <unistd.h>

#include <cassert>

#include "src/event_scheduler/event_scheduler.h"
#include "tool/library_unit_test/library_unit_test.h"

namespace pos
{
IOConfig::IOConfig(uint32_t queueDepth, uint32_t blockSize, uint32_t timeInSecond, bool eventWorker)
: queueDepth(queueDepth),
  blockSize(blockSize),
  eventWorker(eventWorker)
{
    pendingIO = 0;
    timeInNanoseconds = timeInSecond * 1000000000ULL;
    systemTimeoutChecker = nullptr;
}
IOConfig::~IOConfig(void)
{
    if (systemTimeoutChecker != nullptr)
    {
        delete systemTimeoutChecker;
    }
}
void
IOConfig::ExecuteLoop(void)
{
    systemTimeoutChecker = new SystemTimeoutChecker;
    systemTimeoutChecker->SetTimeout(timeInNanoseconds);
    while (systemTimeoutChecker->CheckTimeout() == false)
    {
        if (pendingIO < queueDepth)
        {
            pendingIO++;
            EventSmartPtr event(new DummySubmitHandler(eventWorker, this));
            EventSchedulerSingleton::Instance()->EnqueueEvent(event);
        }
    }
    while (pendingIO > 0)
    {
    }
}

} // namespace pos
