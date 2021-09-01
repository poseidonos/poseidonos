#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/vsamap/vsamap_content.h"

namespace pos
{
class MockVSAMapContent : public VSAMapContent
{
public:
    using VSAMapContent::VSAMapContent;
    MOCK_METHOD(MpageList, GetDirtyPages, (uint64_t start, uint64_t numEntries), (override));
    MOCK_METHOD(VirtualBlkAddr, GetEntry, (BlkAddr rba), (override));
    MOCK_METHOD(int, SetEntry, (BlkAddr rba, VirtualBlkAddr vsa), (override));
    MOCK_METHOD(int64_t, GetNumUsedBlocks, (), (override));
};

} // namespace pos
