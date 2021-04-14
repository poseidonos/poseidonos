#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/meta_file_util.h"

namespace pos
{
class MockMfsMediaToVolume : public MfsMediaToVolume
{
public:
    using MfsMediaToVolume::MfsMediaToVolume;
};

class MockMfsVolumeToMedia : public MfsVolumeToMedia
{
public:
    using MfsVolumeToMedia::MfsVolumeToMedia;
};

class MockMetaFileUtil : public MetaFileUtil
{
public:
    using MetaFileUtil::MetaFileUtil;
};

} // namespace pos
