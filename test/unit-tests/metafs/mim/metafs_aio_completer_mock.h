#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_aio_completer.h"

namespace pos
{
class MockMetaFsAioCompleter : public MetaFsAioCompleter
{
public:
    using MetaFsAioCompleter::MetaFsAioCompleter;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
