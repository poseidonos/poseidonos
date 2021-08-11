#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/log_write_context_factory.h"

namespace pos
{
class MockLogWriteContextFactory : public LogWriteContextFactory
{
public:
    using LogWriteContextFactory::LogWriteContextFactory;
    MOCK_METHOD(void, Init, (JournalConfiguration* config, LogBufferWriteDoneNotifier* target, CallbackSequenceController* sequencer), (override));
    MOCK_METHOD(LogWriteContext*, CreateBlockMapLogWriteContext, (VolumeIoSmartPtr volumeIo, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(LogWriteContext*, CreateStripeMapLogWriteContext, (Stripe* stripe, StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(LogWriteContext*, CreateGcBlockMapLogWriteContext, (GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(std::vector<LogWriteContext*>, CreateGcBlockMapLogWriteContexts, (GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(LogWriteContext*, CreateGcStripeFlushedLogWriteContext, (GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(LogWriteContext*, CreateVolumeDeletedLogWriteContext, (int volId, uint64_t contextVersion, EventSmartPtr callback), (override));
    MOCK_METHOD(LogGroupResetContext*, CreateLogGroupResetContext, (uint64_t offset, int id, uint64_t groupSize, EventSmartPtr callbackEvent, char* dataBuffer), (override));
};

} // namespace pos
