#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/metadata/meta_event_factory.h"

namespace pos
{
class MockMetaEventFactory : public MetaEventFactory
{
public:
    using MetaEventFactory::MetaEventFactory;
    MOCK_METHOD(CallbackSmartPtr, CreateBlockMapUpdateEvent, (VolumeIoSmartPtr volumeIo), (override));
};

} // namespace pos
