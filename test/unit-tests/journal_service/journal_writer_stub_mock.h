#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/journal_writer_stub.h"

namespace pos
{
class MockJournalWriterStub : public JournalWriterStub
{
public:
    using JournalWriterStub::JournalWriterStub;
    MOCK_METHOD(bool, IsEnabled, (), (override));
    MOCK_METHOD(int, AddBlockMapUpdatedLog, (VolumeIoSmartPtr volumeIo, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddStripeMapUpdatedLog, (Stripe * stripe, StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent), (override));
};

} // namespace pos
