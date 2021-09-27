#include <gmock/gmock.h>

#include <utility>

#include "src/device/unvme/unvme_device_context.h"

namespace pos
{
class MockUnvmeDeviceContext : public UnvmeDeviceContext
{
public:
    MOCK_METHOD(void, IncAdminCommandCount, (), (override));
    MOCK_METHOD(void, DecAdminCommandCount, (), (override));
    MOCK_METHOD(bool, IsAdminCommandPendingZero, (), (override));

    // DeviceContext Functions
    MOCK_METHOD(void, IncreasePendingIO, (), (override));
    MOCK_METHOD(void, DecreasePendingIO, (), (override));
    MOCK_METHOD(uint32_t, GetPendingIOCount, (), (override));
    MOCK_METHOD(void, AddPendingError, (IOContext & errorToAdd), (override));
    MOCK_METHOD(void, RemovePendingError, (IOContext & errorToRemove), (override));
    MOCK_METHOD(uint32_t, GetPendingErrorCount, (), (override));
    MOCK_METHOD(IOContext*, GetPendingError, (), (override));
};

} // namespace pos
