#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/include/metafs.h"

namespace pos
{
class MockMetaFs : public MetaFs
{
public:
    using MetaFs::MetaFs;
};

} // namespace pos
