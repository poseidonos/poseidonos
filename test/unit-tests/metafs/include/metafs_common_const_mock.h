#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/metafs_common_const.h"

namespace pos
{
class MockMetaFsCommonConst : public MetaFsCommonConst
{
public:
    using MetaFsCommonConst::MetaFsCommonConst;
};

} // namespace pos
