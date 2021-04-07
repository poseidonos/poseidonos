#include "src/device/device_manager.h"
#include "src/device/mock_device.h"
#include "src/scheduler/event_argument.h"

namespace ibofos {

const uint64_t BLOCK_PER_MOCK_DEVICE = 20UL * 1024 * 1024 * 1024;
const uint64_t BLOCK_PER_NVM_MOCK_DEVICE = 2UL * 1024 * 1024 * 1024;

DeviceManager::DeviceManager(void)
{
}

void DeviceManager::ScanDevs(void)
{
    IOWorker *ioWorker = nullptr;

#if 0
    if (nullptr == ioDispatcher)
    {
        _SetupThreadModel();
    }

    cpu_set_t cpuSet;
    ioWorker = ioDispatcher->AddIOWorker(cpuSet, true);
#else
    UBlockDevice* nvMockDevice 
                    = new MockDevice("nvram", BLOCK_PER_NVM_MOCK_DEVICE);
    devices.push_back(nvMockDevice);
 
    for (int index = 0; index < 8; index++)
    {
        string name = "mock_" + to_string(index);
        UBlockDevice* newMockDevice 
                        = new MockDevice(name.c_str(), BLOCK_PER_MOCK_DEVICE);
        devices.push_back(newMockDevice);
    }
#endif

}

#if 0
void
DeviceManager::_SetupThreadModel(void)
{
    ioDispatcher = new IODispatcher();
    EventArgument::SetStaticMember(nullptr, ioDispatcher); 
}
#endif

IODispatcher *
DeviceManager::GetIODispatcher(void)
{
  //return ioDispatcher;
  return nullptr;
}

void
DeviceManager::PrepareDevice(std::string name)
{
    if (nullptr == GetDev(name))
    {
        UBlockDevice* newMockDevice 
                        = new MockDevice(name.c_str(), BLOCK_PER_MOCK_DEVICE);
        devices.push_back(newMockDevice); 
    }
}


UBlockDevice *DeviceManager::GetDev(std::string name) 
{
    for (auto&l : devices)
    {
        if (l->GetName() == name)
        {
            return l;
        }
    }
    return nullptr;
}


vector<UBlockDevice *> DeviceManager::GetDevs()
{
    return devices;
}

vector<DeviceProperty> DeviceManager::ListDevs()
{
    vector<DeviceProperty> devs;
    
    return devs;
}

}  // namespace ibofos
