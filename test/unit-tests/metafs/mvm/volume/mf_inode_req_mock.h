#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/mf_inode_req.h"

namespace pos
{
class MockMetaFileInodeCreateReq : public MetaFileInodeCreateReq
{
public:
    using MetaFileInodeCreateReq::MetaFileInodeCreateReq;
};

} // namespace pos
