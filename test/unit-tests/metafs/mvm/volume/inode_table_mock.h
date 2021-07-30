#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/inode_table.h"

namespace pos
{
class MockInodeTableContent : public InodeTableContent
{
public:
    using InodeTableContent::InodeTableContent;
};

class MockInodeTable : public InodeTable
{
public:
    using InodeTable::InodeTable;
    MOCK_METHOD(const MetaLpnType, GetLpnCntOfRegion, ());
    MOCK_METHOD(void, Create, (uint32_t maxInodeEntryNum));
    MOCK_METHOD(FileDescriptorType, GetFileDescriptor, (uint32_t inodeIdx));
    MOCK_METHOD(MetaFileInode&, GetInode, (uint32_t idx));
    MOCK_METHOD(MetaFileInodeArray&, GetInodeArray, ());
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
