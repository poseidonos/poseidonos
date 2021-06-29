#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/flush_command_handler.h"

namespace pos
{
class MockFlushCmdHandler : public FlushCmdHandler
{
public:
    using FlushCmdHandler::FlushCmdHandler;
    MOCK_METHOD(bool, Execute, (), (override));
};

class MockMapFlushCompleteEvent : public MapFlushCompleteEvent
{
public:
    using MapFlushCompleteEvent::MapFlushCompleteEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

class MockAllocatorFlushDoneEvent : public AllocatorFlushDoneEvent
{
public:
    using AllocatorFlushDoneEvent::AllocatorFlushDoneEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
