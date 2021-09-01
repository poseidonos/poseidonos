#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_map_manager.h"

namespace pos
{
class MockIMapManagerInternal : public IMapManagerInternal
{
public:
    using IMapManagerInternal::IMapManagerInternal;
    MOCK_METHOD(void, MapFlushDone, (int mapId), (override));
};

} // namespace pos
