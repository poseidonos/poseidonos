#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/mf_inode.h"

namespace pos
{
class MockMetaStorageIoProperty : public MetaStorageIoProperty
{
public:
    using MetaStorageIoProperty::MetaStorageIoProperty;
};

class MockJournalMDPageMap : public JournalMDPageMap
{
public:
    using JournalMDPageMap::JournalMDPageMap;
};

class MockMetaFileInode : public MetaFileInode
{
public:
    using MetaFileInode::MetaFileInode;
};

} // namespace pos
