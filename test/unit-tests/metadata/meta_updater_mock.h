#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metadata/meta_updater.h"

namespace pos
{
class MockMetaUpdater : public MetaUpdater
{
public:
    using MetaUpdater::MetaUpdater;
    MOCK_METHOD(int, UpdateBlockMap, (VolumeIoSmartPtr volumeIo, EventSmartPtr callback), (override));
    MOCK_METHOD(int, UpdateStripeMap, (Stripe* stripe, StripeAddr oldAddr, EventSmartPtr callback), (override));
};

} // namespace pos
