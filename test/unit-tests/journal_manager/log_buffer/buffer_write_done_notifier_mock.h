#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"

namespace pos
{
class MockLogBufferWriteDoneEvent : public LogBufferWriteDoneEvent
{
public:
    using LogBufferWriteDoneEvent::LogBufferWriteDoneEvent;
    MOCK_METHOD(void, LogFilled, (int logGroupId, const MapList& dirty), (override));
    MOCK_METHOD(void, LogBufferReseted, (int logGroupId), (override));
};

class MockLogBufferWriteDoneNotifier : public LogBufferWriteDoneNotifier
{
public:
    using LogBufferWriteDoneNotifier::LogBufferWriteDoneNotifier;
    MOCK_METHOD(void, Register, (LogBufferWriteDoneEvent * notified), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, NotifyLogFilled, (int logGroupId, const MapList& dirty), (override));
    MOCK_METHOD(void, NotifyLogBufferReseted, (int logGroupId), (override));
};

} // namespace pos
