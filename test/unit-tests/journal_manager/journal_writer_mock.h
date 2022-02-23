#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/journal_writer.h"

namespace pos
{
class MockJournalWriter : public JournalWriter
{
public:
    using JournalWriter::JournalWriter;
    MOCK_METHOD(int, Init, (LogWriteHandler* writeHandler, LogWriteContextFactory* logWriteEventFactory, JournalEventFactory* journalEventFactory, JournalingStatus* status), (override));
    MOCK_METHOD(int, AddBlockMapUpdatedLog, (VolumeIoSmartPtr volumeIo, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddStripeMapUpdatedLog, (Stripe* stripe, StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddGcStripeFlushedLog, (GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent), (override));
};

} // namespace pos
