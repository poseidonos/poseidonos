#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <utility>

#include "src/device/uram/uram_io_context.h"

namespace pos
{
class MockUramIOContext : public UramIOContext
{
public:
    MOCK_METHOD(UramDeviceContext*, GetDeviceContext, (), (override));
    MOCK_METHOD(uint32_t, GetRetryCount, ());
    MOCK_METHOD(void, AddRetryCount, ());
    MOCK_METHOD(bool, RequestRetry, (spdk_bdev_io_wait_cb callbackFunc));
    MOCK_METHOD(SpdkBdevCaller*, GetBdevCaller, ());

    // IoContext Functions
    MOCK_METHOD(void, SetErrorKey, (std::list<IOContext*>::iterator it), (override));
    MOCK_METHOD((std::pair<std::list<IOContext*>::iterator, bool>), GetErrorKey, (), (override));
    MOCK_METHOD(std::string, GetDeviceName, (), (override));
    MOCK_METHOD(UbioDir, GetOpcode, (), (override));
    MOCK_METHOD(void*, GetBuffer, (), (override));
    MOCK_METHOD(uint64_t, GetStartByteOffset, (), (override));
    MOCK_METHOD(uint64_t, GetByteCount, (), (override));
    MOCK_METHOD(uint64_t, GetStartSectorOffset, (), (override));
    MOCK_METHOD(uint64_t, GetSectorCount, (), (override));
    MOCK_METHOD(void, AddPendingErrorCount, (uint32_t errorCountToAdd), (override));
    MOCK_METHOD(void, SubtractPendingErrorCount, (uint32_t errorCountToSubtract), (override));
    MOCK_METHOD(void, CompleteIo, (IOErrorType error), (override));
    MOCK_METHOD(void, SetAsyncIOCompleted, (), (override));
    MOCK_METHOD(void, ClearAsyncIOCompleted, (), (override));
    MOCK_METHOD(bool, IsAsyncIOCompleted, (), (override));
    MOCK_METHOD(bool, CheckAndDecreaseErrorRetryCount, (), (override));
    MOCK_METHOD(void, ClearErrorRetryCount, (), (override));
    MOCK_METHOD(void, IncOutOfMemoryRetryCount, (), (override));
    MOCK_METHOD(void, ClearOutOfMemoryRetryCount, (), (override));
    MOCK_METHOD(uint32_t, GetOutOfMemoryRetryCount, (), (override));
};

} // namespace pos
