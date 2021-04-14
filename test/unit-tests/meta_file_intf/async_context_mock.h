#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/meta_file_intf/async_context.h"

namespace pos
{
class MockAsyncMetaFileIoCtx : public AsyncMetaFileIoCtx
{
public:
    using AsyncMetaFileIoCtx::AsyncMetaFileIoCtx;
};

} // namespace pos
