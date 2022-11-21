#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/array_rebuilder.h"

namespace pos
{
class MockArrayRebuilder : public ArrayRebuilder
{
public:
    using ArrayRebuilder::ArrayRebuilder;
    MOCK_METHOD(void, Rebuild,(string array, uint32_t arrayId, vector<IArrayDevice*> dst,
                        RebuildComplete cb, list<RebuildTarget*>& tgt), (override));
    MOCK_METHOD(void, QuickRebuild,(string array, uint32_t arrayId, QuickRebuildPair rebuildPair,
                        RebuildComplete cb, list<RebuildTarget*>& tgt), (override));
    MOCK_METHOD(void, StopRebuild, (string array, int reason), (override));
    MOCK_METHOD(void, RebuildDone, (RebuildResult result), (override));
    MOCK_METHOD(void, WaitRebuildDone, (string array), (override));
    MOCK_METHOD(bool, IsRebuilding, (string array), (override));
    MOCK_METHOD(uint32_t, GetRebuildProgress, (string array), (override));
};

} // namespace pos
