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
};

} // namespace pos
