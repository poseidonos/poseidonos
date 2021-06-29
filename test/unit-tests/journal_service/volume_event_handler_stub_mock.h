#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/volume_event_handler_stub.h"

namespace pos
{
class MockVolumeEventHandlerStub : public VolumeEventHandlerStub
{
public:
    using VolumeEventHandlerStub::VolumeEventHandlerStub;
    MOCK_METHOD(int, VolumeDeleted, (int volId), (override));
};

} // namespace pos
