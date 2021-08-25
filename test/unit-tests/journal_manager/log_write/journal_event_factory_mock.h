#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_write/journal_event_factory.h"

namespace pos
{
class MockJournalEventFactory : public JournalEventFactory
{
public:
    using JournalEventFactory::JournalEventFactory;
    MOCK_METHOD(void, Init, (LogWriteHandler* logWriteHandler), (override));
    MOCK_METHOD(EventSmartPtr, CreateGcLogWriteCompletedEvent, (LogWriteContext* callbackContext), (override));
};

} // namespace pos
