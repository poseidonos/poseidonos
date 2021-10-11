#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/admin/smart_log_meta_io.h"

namespace pos
{
class MockLogPageFlushIoCtx : public LogPageFlushIoCtx
{
public:
    using LogPageFlushIoCtx::LogPageFlushIoCtx;
};

class MockSmartLogMetaIo : public SmartLogMetaIo
{
public:
    using SmartLogMetaIo::SmartLogMetaIo;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
};

} // namespace pos
