#pragma once

#define UNVME_BUILD

#include <atomic>
#include <future>
#include <string>
#include <thread>

#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event.h"
#include "src/lib/system_timeout_checker.h"

namespace pos
{
class IOConfig
{
public:
    IOConfig(uint32_t queueDepth, uint32_t blockSize, uint32_t timeInSeconds, bool eventWorker = true);
    ~IOConfig(void);
    void ExecuteLoop(void);
    virtual void Submit(void) = 0;
    uint32_t queueDepth;
    std::atomic<uint32_t> pendingIO;
    uint32_t blockSize;

private:
    uint64_t timeInNanoseconds;
    SystemTimeoutChecker* systemTimeoutChecker;
    bool eventWorker;
};
class DummySubmitHandler : public Event
{
public:
    DummySubmitHandler(bool isFront, IOConfig* ioConfig)
    : Event(isFront),
      ioConfig(ioConfig)
    {
    }
    ~DummySubmitHandler(void) override{};

    bool
    Execute(void)
    {
        ioConfig->Submit();
        return true;
    }

private:
    IOConfig* ioConfig;
};
class DummyCallbackHandler : public Callback
{
public:
    DummyCallbackHandler(bool isFront, IOConfig* ioConfig)
    : Callback(isFront),
      ioConfig(ioConfig)
    {
    }
    ~DummyCallbackHandler(void) override{};

private:
    IOConfig* ioConfig;
    bool
    _DoSpecificJob(void)
    {
        ioConfig->pendingIO--;
        return true;
    }
};
} // namespace pos
