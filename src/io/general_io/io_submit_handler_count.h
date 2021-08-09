#pragma once
#include <atomic>
#include "src/include/pos_event_id.h"
#include "src/lib/atomic_count.h"
#include "src/lib/singleton.h"
#include "src/logger/logger.h"

namespace pos
{

class AtomicValueIOSubmitHandlerCount : public AtomicCount<int64_t>
{
public:
    explicit AtomicValueIOSubmitHandlerCount(uint64_t value)
    : AtomicCount(value)
    {
    }
    void ErrorLogUnderflow(void) override
    {
        POS_TRACE_WARN(POS_EVENT_ID::DEBUG_ATOMIC_UNDERFLOW,
            "io submit handler count underflow!");
    }
};
class IOSubmitHandlerCount
{
public:
    IOSubmitHandlerCount():
    pendingRead(0),
    pendingWrite(0),
    pendingByteIo(0),
    callbackNotCalledCount(0)
    {
    }
    AtomicValueIOSubmitHandlerCount pendingRead;
    AtomicValueIOSubmitHandlerCount pendingWrite;
    AtomicValueIOSubmitHandlerCount pendingByteIo;
    AtomicValueIOSubmitHandlerCount callbackNotCalledCount;
};
using IOSubmitHandlerCountSingleton = Singleton<IOSubmitHandlerCount>;
} // namespace pos
