#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/journal_writer.h"

namespace pos
{
class MockJournalWriter : public JournalWriter
{
public:
    using JournalWriter::JournalWriter;
    MOCK_METHOD(int, Init, (LogWriteHandler * writeHandler, LogWriteContextFactory* logWriteEventFactory, JournalEventFactory* journalEventFactory, JournalingStatus* status, EventScheduler* scheduler), (override));
    MOCK_METHOD(int, AddBlockMapUpdatedLog, (VolumeIoSmartPtr volumeIo, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddStripeMapUpdatedLog, (Stripe * stripe, StripeAddr oldAddr, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddGcStripeFlushedLog, (GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent), (override));
};

} // namespace pos
