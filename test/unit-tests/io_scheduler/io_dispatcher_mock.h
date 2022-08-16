#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/io_dispatcher.h"

namespace pos
{
class MockIODispatcher : public IODispatcher
{
public:
    using IODispatcher::IODispatcher;
    MOCK_METHOD(void, AddIOWorker, (cpu_set_t cpuSet), (override));
    MOCK_METHOD(void, RemoveIOWorker, (cpu_set_t cpuSet), (override));
    MOCK_METHOD(void, AddDeviceForReactor, (UblockSharedPtr dev), (override));
    MOCK_METHOD(void, RemoveDeviceForReactor, (UblockSharedPtr dev), (override));
    MOCK_METHOD(void, AddDeviceForIOWorker, (UblockSharedPtr dev, cpu_set_t cpuSet), (override));
    MOCK_METHOD(void, RemoveDeviceForIOWorker, (UblockSharedPtr dev), (override));
    MOCK_METHOD(int, Submit, (UbioSmartPtr ubio, bool sync, bool ioRecoveryNeeded), (override));
};

} // namespace pos
