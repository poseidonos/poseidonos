#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_unmount_complete_handler.h"

namespace pos
{
class MockVolumeUnmountCompleteHandler : public VolumeUnmountCompleteHandler
{
public:
    using VolumeUnmountCompleteHandler::VolumeUnmountCompleteHandler;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
