#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/mfs_geometry.h"

namespace pos
{
class MockMetaFsVolumeSpcfInfo : public MetaFsVolumeSpcfInfo
{
public:
    using MetaFsVolumeSpcfInfo::MetaFsVolumeSpcfInfo;
};

class MockMetaFsStorageIoInfo : public MetaFsStorageIoInfo
{
public:
    using MetaFsStorageIoInfo::MetaFsStorageIoInfo;
};

class MockMetaFsGeometryInfo : public MetaFsGeometryInfo
{
public:
    using MetaFsGeometryInfo::MetaFsGeometryInfo;
};

} // namespace pos
