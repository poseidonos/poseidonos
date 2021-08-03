#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/meta_service/i_meta_updater.h"

namespace pos
{
class MockIMetaUpdater : public IMetaUpdater
{
public:
    using IMetaUpdater::IMetaUpdater;
    MOCK_METHOD(int, UpdateBlockMap, (VolumeIoSmartPtr volumeIo, EventSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateStripeMap, (Stripe* stripe, StripeAddr oldAddr, EventSmartPtr callback), (override));
};

} // namespace pos
