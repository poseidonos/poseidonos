#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
class MockVolumeEventPublisher : public VolumeEventPublisher
{
public:
    using VolumeEventPublisher::VolumeEventPublisher;
};

} // namespace pos
