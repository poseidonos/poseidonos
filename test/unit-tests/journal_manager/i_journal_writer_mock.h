#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/i_journal_writer.h"

namespace pos
{
class MockIJournalWriter : public IJournalWriter
{
public:
    using IJournalWriter::IJournalWriter;
    MOCK_METHOD(int, AddBlockMapUpdatedLog, (VolumeIoSmartPtr volumeIo, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddStripeMapUpdatedLog, (Stripe * stripe, StripeAddr oldAddr, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddGcStripeFlushedLog, (GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent), (override));
};

} // namespace pos
