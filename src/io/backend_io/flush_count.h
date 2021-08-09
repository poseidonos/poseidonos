#pragma once
#include <atomic>
#include "src/include/pos_event_id.h"
#include "src/lib/atomic_count.h"
#include "src/lib/singleton.h"
#include "src/logger/logger.h"

namespace pos
{

class AtomicValueFlushCount : public AtomicCount<int64_t>
{
public:
    explicit AtomicValueFlushCount(uint64_t value)
    : AtomicCount(value)
    {
    }
    void ErrorLogUnderflow(void) override
    {
        POS_TRACE_WARN(POS_EVENT_ID::DEBUG_ATOMIC_UNDERFLOW,
            "flush submission count underflow!");
    }
};
class FlushCount
{
public:
    FlushCount(void):
    pendingFlush(0),
    callbackNotCalledCount(0)
    {
    }
    AtomicValueFlushCount pendingFlush;
    AtomicValueFlushCount callbackNotCalledCount;
};
using FlushCountSingleton = Singleton<FlushCount>;
} // namespace pos
