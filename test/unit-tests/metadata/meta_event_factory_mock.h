#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include "src/metadata/meta_event_factory.h"

namespace pos
{
class MockMetaEventFactory : public MetaEventFactory
{
public:
    using MetaEventFactory::MetaEventFactory;
    MOCK_METHOD(CallbackSmartPtr, CreateBlockMapUpdateEvent, (VolumeIoSmartPtr volumeIo), (override));
    MOCK_METHOD(CallbackSmartPtr, CreateStripeMapUpdateEvent, (Stripe* stripe), (override));
    MOCK_METHOD(CallbackSmartPtr, CreateGcMapUpdateEvent, (StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, (std::map<SegmentId, uint32_t> invalidSegCnt)), (override));
};

} // namespace pos
