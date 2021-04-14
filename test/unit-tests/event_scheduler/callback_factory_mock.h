#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/callback_factory.h"

namespace pos
{
class MockCallbackFactory : public CallbackFactory
{
public:
    using CallbackFactory::CallbackFactory;
    MOCK_METHOD(CallbackSmartPtr, Create, (VolumeIoSmartPtr volumeIo), (override));
};

} // namespace pos
