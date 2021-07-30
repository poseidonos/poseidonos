#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/mf_dataformat.h"

namespace pos
{
class MockMetaFileInfoDumpCxt : public MetaFileInfoDumpCxt
{
public:
    using MetaFileInfoDumpCxt::MetaFileInfoDumpCxt;
};

class MockMetaFileInodeInfo : public MetaFileInodeInfo
{
public:
    using MetaFileInodeInfo::MetaFileInodeInfo;
};

class MockMetaFileInodeDumpCxt : public MetaFileInodeDumpCxt
{
public:
    using MetaFileInodeDumpCxt::MetaFileInodeDumpCxt;
};

} // namespace pos
