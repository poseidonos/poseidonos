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
    MOCK_METHOD(void, Init, (LogBufferWriteDoneNotifier * target), (override));
    MOCK_METHOD(LogWriteContext*, CreateBlockMapLogWriteContext, (VolumeIoSmartPtr volumeIo, MpageList dirty, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(LogWriteContext*, CreateStripeMapLogWriteContext, (Stripe * stripe, StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent), (override));
};

} // namespace pos
