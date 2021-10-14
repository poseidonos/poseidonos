#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/journal_event_factory.h"

namespace pos
{
class MockJournalEventFactory : public JournalEventFactory
{
public:
    using JournalEventFactory::JournalEventFactory;
    MOCK_METHOD(void, Init, (EventScheduler * scheduler, LogWriteHandler* logWriteHandler), (override));
    MOCK_METHOD(EventSmartPtr, CreateGcLogWriteCompletedEvent, (EventSmartPtr callback), (override));
    MOCK_METHOD(EventSmartPtr, CreateGcStripeLogWriteRequestEvent, (LogWriteContext * callbackContext), (override));
};

} // namespace pos
