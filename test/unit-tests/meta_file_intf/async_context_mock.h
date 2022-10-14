#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/meta_file_intf/async_context.h"

namespace pos
{
class MockAsyncMetaFileIoCtx : public AsyncMetaFileIoCtx
{
public:
    using AsyncMetaFileIoCtx::AsyncMetaFileIoCtx;
    MOCK_METHOD(void, HandleIoComplete, (void* data), (override));
    MOCK_METHOD(int, GetError, (), (const, override));
    MOCK_METHOD(std::string, ToString, (), (const, override));
};

} // namespace pos
