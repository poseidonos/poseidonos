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
    MOCK_METHOD(bool, Rebuild, (), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
};

} // namespace pos
