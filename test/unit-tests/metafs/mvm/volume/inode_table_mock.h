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
};

} // namespace pos
