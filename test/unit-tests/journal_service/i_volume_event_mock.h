#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_service/i_volume_event.h"

namespace pos
{
class MockIVolumeEventHandler : public IVolumeEventHandler
{
public:
    using IVolumeEventHandler::IVolumeEventHandler;
    MOCK_METHOD(int, VolumeDeleted, (int volId), (override));
};

} // namespace pos
