#include <gmock/gmock.h>

#include <map>

#include "src/meta_service/i_meta_updater.h"

namespace pos
{
class MockIMetaUpdater : public IMetaUpdater
{
public:
    using IMetaUpdater::IMetaUpdater;
    MOCK_METHOD(int, UpdateBlockMap, (VolumeIoSmartPtr volumeIo, CallbackSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateStripeMap, (Stripe * stripe, CallbackSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateGcMap, (StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, (std::map<SegmentId, uint32_t> invalidSegCnt), CallbackSmartPtr callback), (override));
};

} // namespace pos
