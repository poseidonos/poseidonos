#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mfs_io_handler_base.h"

namespace pos
{
class MockMetaFsIoHandlerBase : public MetaFsIoHandlerBase
{
public:
    using MetaFsIoHandlerBase::MetaFsIoHandlerBase;
    MOCK_METHOD(void, StartThread, (), (override));
    MOCK_METHOD(void, ExitThread, (), (override));
};

} // namespace pos
