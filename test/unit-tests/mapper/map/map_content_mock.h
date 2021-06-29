#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map_content.h"

namespace pos
{
class MockMapContent : public MapContent
{
public:
    using MapContent::MapContent;
    MOCK_METHOD(int, Prepare, (uint64_t numEntries, int64_t opt), (override));
    MOCK_METHOD(MpageList, GetDirtyPages, (BlkAddr start, uint64_t numEntries), (override));
};

} // namespace pos
