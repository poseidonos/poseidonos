#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/rebuild/i_array_rebuilder.h"

namespace pos
{
class MockIArrayRebuilder : public IArrayRebuilder
{
public:
    using IArrayRebuilder::IArrayRebuilder;
    MOCK_METHOD(void, Rebuild, (string array, ArrayDevice* dev, RebuildComplete cb, list<RebuildTarget*> tgt), (override));
    MOCK_METHOD(void, StopRebuild, (string array), (override));
    MOCK_METHOD(void, RebuildDone, (string array), (override));
    MOCK_METHOD(void, WaitRebuildDone, (string array), (override));
    MOCK_METHOD(uint32_t, GetRebuildProgress, (string array), (override));
};

} // namespace pos
