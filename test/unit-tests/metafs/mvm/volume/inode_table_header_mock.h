#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/inode_table_header.h"

namespace pos
{
class MockInodeInUseBitmap : public InodeInUseBitmap
{
public:
    using InodeInUseBitmap::InodeInUseBitmap;
};

class MockInodeTableHeaderContent : public InodeTableHeaderContent
{
public:
    using InodeTableHeaderContent::InodeTableHeaderContent;
};

class MockInodeTableHeader : public InodeTableHeader
{
public:
    using InodeTableHeader::InodeTableHeader;
    MOCK_METHOD(const MetaLpnType, GetLpnCntOfRegion, ());
    MOCK_METHOD(void, Create, (uint32_t totalFileNum));
    MOCK_METHOD(void, SetInodeInUse, (uint32_t idx));
    MOCK_METHOD(void, ClearInodeInUse, (uint32_t idx));
    MOCK_METHOD(bool, IsFileInodeInUse, (uint32_t idx));
    MOCK_METHOD(uint32_t, GetTotalAllocatedInodeCnt, ());
    MOCK_METHOD(std::vector<MetaFileExtent>, GetFileExtentContent, ());
    MOCK_METHOD(void, SetFileExtentContent,
        (std::vector<MetaFileExtent>& extents));
    MOCK_METHOD(size_t, GetFileExtentContentSize, ());
    MOCK_METHOD(void, BuildFreeInodeEntryMap, ());
    MOCK_METHOD(uint32_t, GetFreeInodeEntryIdx, ());
    MOCK_METHOD(std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>&,
        GetInodeInUseBitmap, ());
    MOCK_METHOD(bool, Load, ());
    MOCK_METHOD(bool, Load,
        (MetaStorageType media, MetaLpnType baseLPN,
            uint32_t idx, MetaLpnType pageCNT));
    MOCK_METHOD(bool, Store, ());
    MOCK_METHOD(bool, Store,
        (MetaStorageType media, MetaLpnType baseLPN,
            uint32_t idx, MetaLpnType pageCNT));
    MOCK_METHOD(const MetaLpnType, GetBaseLpn, ());
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss));
};

} // namespace pos
