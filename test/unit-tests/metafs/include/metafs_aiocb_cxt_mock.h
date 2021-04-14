#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/metafs_aiocb_cxt.h"

namespace pos
{
class MockMetaFsAioCbCxt : public MetaFsAioCbCxt
{
public:
    using MetaFsAioCbCxt::MetaFsAioCbCxt;
};

} // namespace pos
