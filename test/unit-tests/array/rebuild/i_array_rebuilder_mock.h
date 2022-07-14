#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/rebuild/i_array_rebuilder.h"

namespace pos
{
class MockIArrayRebuilder : public IArrayRebuilder
{
public:
    using IArrayRebuilder::IArrayRebuilder;
    MOCK_METHOD(void, Rebuild, (string array, uint32_t id, ArrayDevice* dev, RebuildComplete cb, list<RebuildTarget*>& tgt, RebuildTypeEnum rebuildType, bool isWT), (override));
    MOCK_METHOD(void, StopRebuild, (string array), (override));
    MOCK_METHOD(void, RebuildDone, (RebuildResult result), (override));
    MOCK_METHOD(void, WaitRebuildDone, (string array), (override));
    MOCK_METHOD(bool, IsRebuilding, (string array), (override));
    MOCK_METHOD(uint32_t, GetRebuildProgress, (string array), (override));
};

} // namespace pos
