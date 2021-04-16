#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/journal_manager.h"

namespace pos
{
class MockJournalManager : public JournalManager
{
public:
    using JournalManager::JournalManager;
    MOCK_METHOD(bool, IsEnabled, (), (override));
    MOCK_METHOD(int, AddBlockMapUpdatedLog, (VolumeIoSmartPtr volumeIo, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddStripeMapUpdatedLog, (Stripe * stripe, StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, AddGcStripeFlushedLog, (int volumeId, GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
};

} // namespace pos
