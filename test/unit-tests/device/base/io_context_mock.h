#include <gmock/gmock.h>

#include <list>
#include <string>
#include <utility>

#include "src/device/base/io_context.h"

namespace pos
{
class MockIOContext : public IOContext
{
public:
    MOCK_METHOD(void, SetErrorKey, (std::list<IOContext*>::iterator it));
    MOCK_METHOD((std::pair<std::list<IOContext*>::iterator, bool>), GetErrorKey, ());
    MOCK_METHOD(std::string, GetDeviceName, ());
    MOCK_METHOD(UbioDir, GetOpcode, ());
    MOCK_METHOD(void*, GetBuffer, ());
    MOCK_METHOD(uint64_t, GetStartByteOffset, ());
    MOCK_METHOD(uint64_t, GetByteCount, ());
    MOCK_METHOD(uint64_t, GetStartSectorOffset, ());
    MOCK_METHOD(uint64_t, GetSectorCount, ());
    MOCK_METHOD(void, AddPendingErrorCount, (uint32_t errorCountToAdd));
    MOCK_METHOD(void, SubtractPendingErrorCount, (uint32_t errorCountToSubtract));
    MOCK_METHOD(bool, CheckErrorDisregard, ());
    MOCK_METHOD(void, CompleteIo, (IOErrorType error));
    MOCK_METHOD(void, SetAsyncIOCompleted, ());
    MOCK_METHOD(void, ClearAsyncIOCompleted, ());
    MOCK_METHOD(bool, IsAsyncIOCompleted, ());
    MOCK_METHOD(bool, CheckAndDecreaseRetryCount, ());
    MOCK_METHOD(void, ClearRetryCount, ());
    MOCK_METHOD(void, IncSubmitRetryCount, ());
    MOCK_METHOD(void, ClearSubmitRetryCount, ());
    MOCK_METHOD(uint32_t, GetSubmitRetryCount, ());
};

} // namespace pos
