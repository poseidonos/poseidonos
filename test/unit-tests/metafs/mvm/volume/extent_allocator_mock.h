#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/extent_allocator.h"

namespace pos
{
class MockExtentAllocator : public ExtentAllocator
{
public:
    using ExtentAllocator::ExtentAllocator;
    MOCK_METHOD(void, Init, (MetaLpnType _base, MetaLpnType _last));
    MOCK_METHOD(std::vector<MetaFileExtent>, AllocExtents, (MetaLpnType lpnCnt));
    MOCK_METHOD(MetaLpnType, GetAvailableLpnCount, ());
    MOCK_METHOD(MetaLpnType, GetAvailableSpace, ());
    MOCK_METHOD(std::vector<MetaFileExtent>, GetAllocatedExtentList, ());
    MOCK_METHOD(void, SetAllocatedExtentList, (std::vector<MetaFileExtent>& list));
    MOCK_METHOD(void, SetFileBaseLpn, (MetaLpnType BaseLpn));
    MOCK_METHOD(MetaLpnType, GetFileBaseLpn, ());
    MOCK_METHOD(bool, AddToFreeList, (MetaLpnType startLpn, MetaLpnType count));
    MOCK_METHOD(void, PrintFreeExtentsList, ());
};

} // namespace pos
