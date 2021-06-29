#include "src/device/device_manager.h"
#include "src/device/mock_device.h"
#include "src/event_scheduler/event_argument.h"

namespace pos {

const uint64_t BLOCK_PER_MOCK_DEVICE = 2 * 1024 * 1024;

DeviceManager::DeviceManager(void)
: ioDispatcher(nullptr),
  eventScheduler(nullptr),
  reactorRegistered(false)
{
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::ScanDevs(void)
{
    IOWorker *ioWorker = nullptr;

    if (nullptr == ioDispatcher)
    {
        _SetupThreadModel();
    }

    cpu_set_t cpuSet;
    ioWorker = ioDispatcher->AddIOWorker(cpuSet, true);
}

void
DeviceManager::_SetupThreadModel(void)
{
    ioDispatcher = new IODispatcher();
    EventArgument::SetStaticMember(nullptr, ioDispatcher); 
    
}

IODispatcher *
DeviceManager::GetIODispatcher(void)
{
  return ioDispatcher;
}

void
DeviceManager::PrepareDevice(std::string name)
{
    if (nullptr == GetDevByName(name))
    {
        UBlockDevice* newMockDevice 
                        = new MockDevice(name.c_str(), BLOCKS_PER_MOCK_DEVICE);
        devices.push_back(newMockDevice); 
    }
}


UBlockDevice *DeviceManager::GetDevByName(std::string name) 
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
    vector<UBlockDevice *> devs;
    return devs;
}

vector<DeviceProperty> DeviceManager::ListDevs()
{
    vector<DeviceProperty> devs;
    
    return devs;
}

}  // namespace pos
