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
    MOCK_METHOD(int, UpdateStripeMap, (StripeSmartPtr stripe, CallbackSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateGcMap, (StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, (std::map<SegmentId, uint32_t> invalidSegCnt), CallbackSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateFreedSegmentContext, (SegmentCtx* segmentCtx, SegmentId targetSegmentId), (override));
};

} // namespace pos
