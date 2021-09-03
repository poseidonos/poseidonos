#include <gmock/gmock.h>

#include <map>

#include "src/metadata/meta_event_factory.h"

namespace pos
{
class MockMetaEventFactory : public MetaEventFactory
{
public:
    using MetaEventFactory::MetaEventFactory;
    MOCK_METHOD(CallbackSmartPtr, CreateBlockMapUpdateEvent, (VolumeIoSmartPtr volumeIo), (override));
    MOCK_METHOD(CallbackSmartPtr, CreateGcMapUpdateEvent, (Stripe * stripe, GcStripeMapUpdateList mapUpdateInfoList, (std::map<SegmentId, uint32_t> invalidSegCnt)), (override));
};

} // namespace pos
