#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/metafs.h"

namespace pos
{
class MockMetaFs : public MetaFs
{
public:
    using MetaFs::MetaFs;
    MOCK_METHOD(uint64_t, GetEpochSignature, (), (override));
};

} // namespace pos
