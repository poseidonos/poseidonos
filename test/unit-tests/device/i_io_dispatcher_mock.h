#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/i_io_dispatcher.h"

namespace pos
{
class MockIIODispatcher : public IIODispatcher
{
public:
    using IIODispatcher::IIODispatcher;
    MOCK_METHOD(void, AddIOWorker, (cpu_set_t cpuSet), (override));
    MOCK_METHOD(void, RemoveIOWorker, (cpu_set_t cpuSet), (override));
    MOCK_METHOD(void, AddDeviceForReactor, (UblockSharedPtr dev), (override));
    MOCK_METHOD(void, RemoveDeviceForReactor, (UblockSharedPtr dev), (override));
    MOCK_METHOD(void, AddDeviceForIOWorker, (UblockSharedPtr dev, cpu_set_t cpuSet), (override));
    MOCK_METHOD(void, RemoveDeviceForIOWorker, (UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Submit, (UbioSmartPtr ubio, bool sync, bool ioRecoveryNeeded), (override));
    MOCK_METHOD(void, ProcessQueues, (), (override));
};

} // namespace pos
