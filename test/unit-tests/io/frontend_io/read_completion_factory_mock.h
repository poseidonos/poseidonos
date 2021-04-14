#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/read_completion_factory.h"

namespace pos
{
class MockReadCompletionFactory : public ReadCompletionFactory
{
public:
    using ReadCompletionFactory::ReadCompletionFactory;
    MOCK_METHOD(CallbackSmartPtr, Create, (VolumeIoSmartPtr volumeIo), (override));
};

} // namespace pos
