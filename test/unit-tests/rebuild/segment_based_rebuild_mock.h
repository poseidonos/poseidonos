#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/segment_based_rebuild.h"

namespace pos
{
class MockSegmentBasedRebuild : public SegmentBasedRebuild
{
public:
    using SegmentBasedRebuild::SegmentBasedRebuild;
    MOCK_METHOD(bool, Init, (), (override));
    MOCK_METHOD(bool, Read, (), (override));
    MOCK_METHOD(bool, Write, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(bool, Complete, (uint32_t targetId, UbioSmartPtr ubio), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
    MOCK_METHOD(string, _GetClassName, (), (override));
};

} // namespace pos
