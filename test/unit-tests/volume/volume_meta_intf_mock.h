#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_meta_intf.h"

namespace pos
{
class MockVolumeMetaIntf : public VolumeMetaIntf
{
public:
    using VolumeMetaIntf::VolumeMetaIntf;
};

} // namespace pos
