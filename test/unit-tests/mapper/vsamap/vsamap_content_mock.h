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
    MOCK_METHOD(int, Prepare, (uint64_t size, int64_t opt), (override));
    MOCK_METHOD(MpageList, GetDirtyPages, (uint64_t start, uint64_t numEntries), (override));
};

} // namespace pos
