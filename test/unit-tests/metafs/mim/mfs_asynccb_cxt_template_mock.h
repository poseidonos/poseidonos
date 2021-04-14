#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mfs_asynccb_cxt_template.h"

namespace pos
{
class MockMetaAsyncCbCxt : public MetaAsyncCbCxt
{
public:
    using MetaAsyncCbCxt::MetaAsyncCbCxt;
};

} // namespace pos
