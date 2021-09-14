#include <gmock/gmock.h>

#include <list>
#include <string>
#include <utility>

#include "src/device/unvme/unvme_io_context.h"

namespace pos
{
class MockUnvmeIOContext : public UnvmeIOContext
{
public:
    MOCK_METHOD(UnvmeDeviceContext*, GetDeviceContext, (), (override));
    MOCK_METHOD(bool, IsFrontEnd, (), (override));
    MOCK_METHOD(void, SetAdminCommand, (), (override));
    MOCK_METHOD(bool, IsAdminCommand, (), (override));

    // IoContext Functions
    MOCK_METHOD(void, SetIOKey, (std::list<IOContext*>::iterator it), (override));
    MOCK_METHOD((std::pair<std::list<IOContext*>::iterator, bool>), GetIOKey, (), (override));
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
    MOCK_METHOD(bool, CheckErrorDisregard, (), (override));
    MOCK_METHOD(void, CompleteIo, (IOErrorType error), (override));
    MOCK_METHOD(void, SetAsyncIOCompleted, (), (override));
    MOCK_METHOD(void, ClearAsyncIOCompleted, (), (override));
    MOCK_METHOD(bool, IsAsyncIOCompleted, (), (override));
    MOCK_METHOD(bool, CheckAndDecreaseRetryCount, (), (override));
    MOCK_METHOD(void, ClearRetryCount, (), (override));
    MOCK_METHOD(void, IncSubmitRetryCount, (), (override));
    MOCK_METHOD(void, ClearSubmitRetryCount, (), (override));
    MOCK_METHOD(uint32_t, GetSubmitRetryCount, (), (override));
};

} // namespace pos
