/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "device_manager.h"

#include <algorithm>
#include <future>
#include <string>
#include <vector>

#include "device_driver.h"
#include "event_framework_api.h"
#include "spdk/nvme.hpp"
#include "src/array/array.h"
#include "src/io/general_io/affinity_manager.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"
#include "src/scheduler/io_dispatcher.h"
#include "unvme/unvme_drv.h"
#include "unvme/unvme_ssd.h"
#include "unvram/unvram_drv.h"

namespace ibofos
{
void
DeviceDetachEventHandler(string sn)
{
    DevUid uid(sn);
    DeviceManagerSingleton::Instance()->DetachDevice(uid);
}

void
DeviceAttachEventHandler(UBlockDevice* dev)
{
    if (dev == nullptr)
    {
        return;
    }

    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_ATTACH,
        "DeviceAttachEventHandler: {}", dev->GetName());
    DeviceManagerSingleton::Instance()->AttachDevice(dev);
}

void
MonitorStart(DeviceMonitor* monitor)
{
    monitor->Start();
}

DeviceManager::DeviceManager(void)
: eventScheduler(nullptr),
  affinityManager(AffinityManagerSingleton::Instance()),
  reactorRegistered(false)
{
    if (nullptr == eventScheduler)
    {
        _SetupThreadModel();
    }
    _InitDriver();
    _InitMonitor();
}

DeviceManager::~DeviceManager()
{
    _ClearDevices();
    _ClearMonitors();
    _ClearWorkers();
}

void
DeviceManager::_ClearWorkers()
{
    delete eventScheduler;
    delete EventArgument::GetIODispatcher();
}
void
DeviceManager::_InitDriver()
{
    drivers.push_back(UnvmeDrvSingleton::Instance());
    drivers.push_back(UnvramDrvSingleton::Instance());
}

void
DeviceManager::_InitMonitor()
{
    UnvmeDrvSingleton::Instance()->SetDeviceEventCallback(DeviceAttachEventHandler, DeviceDetachEventHandler);

    monitors.push_back(UnvmeDrvSingleton::Instance()->GetDaemon());
}

int
DeviceManager::IterateDevicesAndDoFunc(DeviceIterFunc func, void* ctx)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    if (devices.size() == 0)
    {
        return NO_DEVICE_ERROR;
    }
    for (auto it = devices.begin(); it != devices.end(); it++)
    {
        func(*it, ctx);
    }
    return 0;
}

// _ClearDevices() - DO NOT CALL EXCEPT DESTRUCTOR
void
DeviceManager::_ClearDevices()
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    for (auto it = devices.begin(); it != devices.end(); it++)
    {
        UBlockDevice* dev = *it;

        IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
        ioDispatcher->RemoveDeviceForReactor(dev);
        ioDispatcher->RemoveDeviceForIOWorker(dev);

        delete dev;
    }
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_CLEAR_DEVICE, "devices has been cleared sucessfully");
    devices.clear();
}

void
DeviceManager::_ClearMonitors()
{
    for (auto it = monitors.begin(); it != monitors.end(); it++)
    {
        if ((*it)->IsRunning() == true)
        {
            (*it)->Stop();
        }
    }
    monitors.clear();
}

void
DeviceManager::StartMonitoring()
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_START_MONITOR,
        "Start Monitoring");

    bool tryStart = false;
    for (auto it = monitors.begin(); it != monitors.end(); ++it)
    {
        if ((*it)->IsRunning() == false)
        {
            auto f = async(launch::async, MonitorStart, (*it));
            monitorFutures.push_back(std::move(f));
            tryStart = true;
        }
    }

    if (tryStart == false)
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::DEVICEMGR_START_MONITOR,
            "unable to start monitoring");
    }
}

void
DeviceManager::StopMonitoring()
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_STOP_MONITOR, "StopMonitoring");

    bool tryStop = false;
    for (auto it = monitors.begin(); it != monitors.end(); ++it)
    {
        if ((*it)->IsRunning() == true)
        {
            (*it)->Stop();
            tryStop = true;
        }
    }
    if (tryStop == false)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::DEVICEMGR_STOP_MONITOR,
            "unable to stop monitoring");
    }
}

vector<pair<string, string>>
DeviceManager::MonitoringState()
{
    vector<pair<string, string>> result;
    for (auto it = monitors.begin(); it != monitors.end(); ++it)
    {
        string state = "stopped";
        if ((*it)->IsRunning())
        {
            state = "running";
        }

        result.push_back(make_pair((*it)->GetName(), state));
    }

    return result;
}

bool
DeviceManager::_CheckDuplication(UBlockDevice* dev)
{
    DevName name(dev->GetName());
    DevUid sn(dev->GetSN());

    if (GetDev(name) == nullptr && GetDev(sn) == nullptr)
    {
        return true;
    }

    IBOF_TRACE_WARN(IBOF_EVENT_ID::DEVICEMGR_CHK_DUPLICATE,
        "DEVICE DUPLICATED Name:{}, SN: {}",
        name.val, sn.val);

    return false;
}

void
DeviceManager::ScanDevs(void)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    if (devices.size() == 0)
    {
        _PrepareIOWorker();
        _InitScan();
        _PrepareDevices();
        StartMonitoring();
    }
    else
    {
        _Rescan();
    }
}

void
DeviceManager::_InitScan()
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_INIT_SCAN, "InitScan begin");
    for (size_t i = 0; i < drivers.size(); i++)
    {
        IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_INIT_SCAN,
            "scanning {}, {}", i, drivers[i]->GetName());
        vector<UBlockDevice*> _devs;
        drivers[i]->ScanDevs(&_devs);

        for (size_t j = 0; j < _devs.size(); j++)
        {
            if (_CheckDuplication(_devs[j]) == true)
            {
                devices.push_back(_devs[j]);
            }
            else
            {
                delete _devs[j];
            }
        }
    }
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_INIT_SCAN,
        "InitScan done, dev cnt: {}", devices.size());
}

int
DeviceManager::RemoveDevice(UBlockDevice* dev)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    auto iter = find(devices.begin(), devices.end(), dev);
    if (iter == devices.end())
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::DEVICEMGR_REMOVE_DEV,
            "device not found");
        return static_cast<int>(IBOF_EVENT_ID::DEVICEMGR_REMOVE_DEV);
    }

    UnvmeSsd* ssd = nullptr;

    DeviceType type = (*iter)->GetType();
    if (type == DeviceType::SSD)
    {
        ssd = dynamic_cast<UnvmeSsd*>(dev);
    }

    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
    ioDispatcher->RemoveDeviceForReactor(dev);
    ioDispatcher->RemoveDeviceForIOWorker(dev);

    devices.erase(iter);
    if (type == DeviceType::SSD && ssd != nullptr)
    {
        UnvmeDrvSingleton::Instance()->GetDaemon()->Resume();
    }

    IBOF_TRACE_WARN(IBOF_EVENT_ID::DEVICEMGR_REMOVE_DEV,
        "device removed successfully {}", dev->GetName());
    delete dev;

    return 0;
}

void
DeviceManager::_Rescan()
{
    // UNVRAM RESCAN
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_RESCAN, "ReScan begin");
    size_t i;
    for (i = 0; i < drivers.size(); i++)
    {
        if (drivers[i]->GetName() == "UnvramDrv")
        {
            break;
        }
    }

    if (i >= drivers.size())
    {
        // NO UNVRAM_DRV
        IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_RESCAN,
            "ReScan done, dev cnt: {}", devices.size());
        return;
    }

    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::DEVICEMGR_RESCAN,
        "scanning {}, {}", i, drivers[i]->GetName());
    vector<UBlockDevice*> _devs;
    drivers[i]->ScanDevs(&_devs);

    std::vector<UBlockDevice*>::iterator it;
    for (it = devices.begin(); it != devices.end(); it++)
    {
        if ((*it)->GetType() != DeviceType::NVRAM)
        {
            continue;
        }

        bool isExist = false;
        for (size_t k = 0; k < _devs.size(); k++)
        {
            if ((*it)->GetSN() == _devs[k]->GetSN())
            {
                isExist = true;
            }
        }

        if (isExist == false)
        {
            _DetachDeviceImpl(*it);
        }
    }

    for (size_t j = 0; j < _devs.size(); j++)
    {
        if (_CheckDuplication(_devs[j]) == true)
        {
            devices.push_back(_devs[j]);
            _PrepareDevice(_devs[j]);
        }
        else
        {
            delete _devs[j];
        }
    }

    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_RESCAN,
        "ReScan done, dev cnt: {}", devices.size());
}

UBlockDevice*
DeviceManager::GetDev(DeviceIdentifier& devID)
{
    auto it = find_if(devices.begin(), devices.end(), devID.GetPredicate());
    if (it != devices.end())
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::DEVICEMGR_GETDEV,
            "Device Found: {}", devID.val);
        return (*it);
    }

    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_GETDEV,
        "Device Not Found: {}", devID.val);
    return nullptr;
}

vector<UBlockDevice*>
DeviceManager::GetDevs()
{
    return devices;
}

vector<DeviceProperty>
DeviceManager::ListDevs()
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    vector<DeviceProperty> devs;
    for (size_t i = 0; i < devices.size(); i++)
    {
        devs.push_back(devices[i]->GetProperty());
    }

    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_LISTDEV,
        "ListDevs, count: {}", devs.size());
    return devs;
}

int
DeviceManager::PassThroughNvmeAdminCommand(std::string& deviceName,
    struct spdk_nvme_cmd* cmd, void* buffer, uint32_t bufferSizeInBytes)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    int errorCode = -1;
    for (UBlockDevice* dev : devices)
    {
        if (0 == static_cast<string>(dev->GetName()).compare(deviceName))
        {
            UnvmeSsd* unvmeSsd = static_cast<UnvmeSsd*>(dev);
            errorCode = unvmeSsd->PassThroughNvmeAdminCommand(cmd,
                buffer, bufferSizeInBytes);
            break;
        }
    }

    return errorCode;
}

struct spdk_nvme_ctrlr*
DeviceManager::GetNvmeCtrlr(std::string& deviceName)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    for (UBlockDevice* dev : devices)
    {
        DeviceType deviceType = dev->GetType();
        if (((DeviceType::SSD == deviceType) || (DeviceType::ZSSD == deviceType)) &&
            (0 == static_cast<string>(dev->GetName()).compare(deviceName)))
        {
            UnvmeSsd* unvmeSsd = static_cast<UnvmeSsd*>(dev);
            return spdk_nvme_ns_get_ctrlr(unvmeSsd->GetNs());
        }
    }

    return nullptr;
}

void
DeviceManager::AttachDevice(UBlockDevice* dev)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    if (_CheckDuplication(dev) == true)
    {
        _PrepareDevice(dev);
        devices.push_back(dev);
        IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_ATTACH,
            "AttachDevice - add a new device to the system: {}",
            dev->GetName());
    }
    else
    {
        delete dev;
    }
}

int
DeviceManager::DetachDevice(DevUid uid)
{
    int ret = 0;
    bool lockAcquiredFailed = false;

    do
    {
        {
            std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
            UBlockDevice* dev = GetDev(uid);
            if (dev == nullptr)
            {
                // Device Manager can call DetachDevice for already removed Ublock Device.
                // So, just resume the Daemon.
                UnvmeDrvSingleton::Instance()->GetDaemon()->Resume();
                IBOF_TRACE_WARN(IBOF_EVENT_ID::DEVICEMGR_DETACH,
                    "DetachDevice - unknown device or already detached: {}", uid.val);
                return static_cast<int>(IBOF_EVENT_ID::DEVICEMGR_DETACH);
            }
            ret = _DetachDeviceImpl(dev);
        }

        lockAcquiredFailed = (ret == LOCK_ACQUIRE_FAILED);
        if (lockAcquiredFailed)
        {
            usleep(1);
        }
    } while (lockAcquiredFailed);
    return ret;
}

int
DeviceManager::_DetachDeviceImpl(UBlockDevice* dev)
{
    IBOF_TRACE_INFO(IBOF_EVENT_ID::DEVICEMGR_DETACH,
        "DetachDevice: {}", dev->GetName());
    if (dev->GetClass() == DeviceClass::ARRAY)
    {
        if (ArraySingleton::Instance()->DetachDevice(dev) < 0)
        {
            return LOCK_ACQUIRE_FAILED;
        }
    }
    else
    {
        RemoveDevice(dev);
    }

    return 0;
}

void
DeviceManager::_PrepareIOWorker(void)
{
    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
    cpu_set_t targetCpuSet =
        affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
    ioDispatcher->AddIOWorker(targetCpuSet);
}

void
DeviceManager::_PrepareDevices(void)
{
    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
    for (auto& iter : devices)
    {
        ioDispatcher->AddDeviceForReactor(iter);
        cpu_set_t targetCpuSet =
            affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
        ioDispatcher->AddDeviceForIOWorker(iter, targetCpuSet);
    }
}

void
DeviceManager::_PrepareDevice(UBlockDevice* dev)
{
    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
    ioDispatcher->AddDeviceForReactor(dev);
    cpu_set_t cpuSet = affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
    ioDispatcher->AddDeviceForIOWorker(dev, cpuSet);
    dev->Open();
}

void
DeviceManager::_PrepareMockDevice(UBlockDevice* dev)
{
    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
    cpu_set_t cpuSet = affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
    ioDispatcher->AddDeviceForIOWorker(dev, cpuSet);
}

void
DeviceManager::HandleCompletedCommand(void)
{
    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
    ioDispatcher->CompleteForThreadLocalDeviceList();
}
void
DeviceManager::_SetupThreadModel(void)
{
    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::DEVICEMGR_SETUPMODEL, "_SetupThreadModel");
    uint32_t eventCoreCount =
        affinityManager->GetCoreCount(CoreType::EVENT_WORKER);
    uint32_t eventWorkerCount = eventCoreCount * EVENT_THREAD_CORE_RATIO;
    cpu_set_t schedulerCPUSet =
        affinityManager->GetCpuSet(CoreType::EVENT_SCHEDULER);
    cpu_set_t eventCPUSet = affinityManager->GetCpuSet(CoreType::EVENT_WORKER);

    eventScheduler = new ibofos::EventScheduler(eventWorkerCount,
        schedulerCPUSet,
        eventCPUSet);
    IODispatcher* ioDispatcher = new IODispatcher();
    EventArgument::SetStaticMember(eventScheduler, ioDispatcher);
}

} // namespace ibofos
