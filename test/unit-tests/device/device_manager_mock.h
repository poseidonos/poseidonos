#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/device_manager.h"

namespace pos
{
class MockDeviceManager : public DeviceManager
{
public:
    using DeviceManager::DeviceManager;
    MOCK_METHOD(void, Initialize, (IIODispatcher * ioDispatcherInterface), (override));
    MOCK_METHOD(void, ScanDevs, (), (override));
    MOCK_METHOD(UblockSharedPtr, GetDev, (DeviceIdentifier & devID), (override));
    MOCK_METHOD(vector<UblockSharedPtr>, GetDevs, (), (override));
    MOCK_METHOD(vector<DeviceProperty>, ListDevs, (), (override));
    MOCK_METHOD(void, AttachDevice, (UblockSharedPtr dev), (override));
    MOCK_METHOD(int, DetachDevice, (DevUid sn), (override));
    MOCK_METHOD(int, RemoveDevice, (UblockSharedPtr dev), (override));
    MOCK_METHOD(struct spdk_nvme_ctrlr*, GetNvmeCtrlr, (std::string & deviceName), (override));
    MOCK_METHOD(int, PassThroughNvmeAdminCommand, (std::string & deviceName, struct spdk_nvme_cmd* cmd, void* buffer, uint32_t bufferSizeInBytes), (override));
    MOCK_METHOD(void, StartMonitoring, (), (override));
    MOCK_METHOD(void, StopMonitoring, (), (override));
    MOCK_METHOD((vector<pair<string, string>>), MonitoringState, (), (override));
    MOCK_METHOD(void, HandleCompletedCommand, (), (override));
    MOCK_METHOD(int, IterateDevicesAndDoFunc, (DeviceIterFunc func, void* ctx), (override));
    MOCK_METHOD(void, SetDeviceEventCallback, (IDeviceEvent * event), (override));
};

} // namespace pos
