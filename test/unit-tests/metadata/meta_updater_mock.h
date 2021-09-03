#include <gmock/gmock.h>

#include <map>

#include "src/metadata/meta_updater.h"

namespace pos
{
class MockMetaUpdater : public MetaUpdater
{
public:
    using MetaUpdater::MetaUpdater;
    MOCK_METHOD(int, UpdateBlockMap, (VolumeIoSmartPtr volumeIo, CallbackSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateStripeMap, (Stripe * stripe, CallbackSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateGcMap, (Stripe * stripe, GcStripeMapUpdateList mapUpdateInfoList, (std::map<SegmentId, uint32_t> invalidSegCnt), CallbackSmartPtr callback), (override));
};

} // namespace pos
