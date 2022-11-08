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
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
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

#include "i_io_dispatcher.h"
#include "src/bio/ubio.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/nvme.hpp"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/base/device_driver.h"
#include "src/include/branch_prediction.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "unvme/unvme_drv.h"
#include "unvme/unvme_ssd.h"
#include "uram/uram_drv.h"

namespace pos
{
void
DeviceDetachEventHandler(string sn)
{
    DevUid uid(sn);
    DeviceManagerSingleton::Instance()->DetachDevice(uid);
}

void
DeviceAttachEventHandler(UblockSharedPtr dev)
{
    if (dev == nullptr)
    {
        return;
    }

    POS_TRACE_INFO(EID(DEVICEMGR_ATTACH),
        "DeviceAttachEventHandler: {}", dev->GetName());
    DeviceManagerSingleton::Instance()->AttachDevice(dev);
}

void
MonitorStart(DeviceMonitor* monitor)
{
    monitor->Start();
}

DeviceManager::DeviceManager(AffinityManager* affinityManager,
    SpdkNvmeCaller* spdkNvmeCaller)
: affinityManager(affinityManager),
  spdkNvmeCaller(spdkNvmeCaller)
{
    _InitDriver();
    _InitMonitor();
}

DeviceManager::~DeviceManager()
{
    _ClearDevices();
    _ClearMonitors();
    if (spdkNvmeCaller != nullptr)
    {
        delete spdkNvmeCaller;
    }
}

void
DeviceManager::_InitDriver()
{
    drivers.push_back(UnvmeDrvSingleton::Instance());
    drivers.push_back(UramDrvSingleton::Instance());
}

void
DeviceManager::_InitMonitor()
{
    UnvmeDrvSingleton::Instance()->SetDeviceEventCallback(DeviceAttachEventHandler, DeviceDetachEventHandler);

    monitors.push_back(UnvmeDrvSingleton::Instance()->GetDaemon());
}

void
DeviceManager::Initialize(IIODispatcher* ioDispatcherInterface)
{
    ioDispatcher = ioDispatcherInterface;
}

int
DeviceManager::IterateDevicesAndDoFunc(DeviceIterFunc func, void* ctx)
{
    vector<UblockSharedPtr> devicesLocal;
    devicesLocal.clear();
    // Local Copy of device managers increase ublock's reference count.
    // Even if a single device is removed from device list, it still must be maintained in memory.
    // If func uses device lock, this implementation will avoid dead lock condition.
    {
        std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
        if (devices.size() == 0)
        {
            int result = EID(DEVICEMGR_DEVICE_NOT_FOUND);
            POS_TRACE_ERROR(result, "There is no device");
            return result;
        }
        for (auto it = devices.begin(); it != devices.end(); it++)
        {
            devicesLocal.push_back(*it);
        }
    }

    for (auto it = devicesLocal.begin(); it != devicesLocal.end(); it++)
    {
        func(*it, ctx);
    }

    devicesLocal.clear();
    return 0;
}

void
DeviceManager::SetDeviceEventCallback(IDeviceEvent* event)
{
    deviceEvent = event;
}

// _ClearDevices() - DO NOT CALL EXCEPT DESTRUCTOR
void
DeviceManager::_ClearDevices()
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    for (UblockSharedPtr dev : devices)
    {
        ioDispatcher->RemoveDeviceForReactor(dev);
        ioDispatcher->RemoveDeviceForIOWorker(dev);
        dev.reset();
    }
    POS_TRACE_INFO(EID(DEVICEMGR_CLEAR_DEVICE), "devices has been cleared sucessfully");
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
DeviceManager::_StartMonitoring()
{
    POS_TRACE_INFO(EID(DEVICEMGR_START_MONITOR),
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
        POS_TRACE_ERROR(EID(DEVICEMGR_START_MONITOR),
            "unable to start monitoring");
    }
}

bool
DeviceManager::_CheckDuplication(UblockSharedPtr dev)
{
    DevName name(dev->GetName());
    DevUid sn(dev->GetSN());

    if (GetDev(name) == nullptr && GetDev(sn) == nullptr)
    {
        return true;
    }

    POS_TRACE_WARN(EID(DEVICEMGR_CHK_DUPLICATE),
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
        _StartMonitoring();
    }
    else
    {
        _Rescan();
    }
}

void
DeviceManager::_InitScan()
{
    POS_TRACE_INFO(EID(DEVICEMGR_INIT_SCAN), "InitScan begin");
    for (size_t i = 0; i < drivers.size(); i++)
    {
        POS_TRACE_INFO(EID(DEVICEMGR_INIT_SCAN),
            "scanning {}, {}", i, drivers[i]->GetName());
        vector<UblockSharedPtr> _devs;
        drivers[i]->ScanDevs(&_devs);

        for (size_t j = 0; j < _devs.size(); j++)
        {
            if (_CheckDuplication(_devs[j]) == true)
            {
                devices.push_back(_devs[j]);
            }
        }
    }
    POS_TRACE_INFO(EID(DEVICEMGR_INIT_SCAN),
        "InitScan done, dev cnt: {}", devices.size());
}

int
DeviceManager::RemoveDevice(UblockSharedPtr dev)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    auto iter = find(devices.begin(), devices.end(), dev);
    if (iter == devices.end())
    {
        POS_TRACE_ERROR(EID(DEVICEMGR_REMOVE_DEV),
            "device not found");
        return static_cast<int>(EID(DEVICEMGR_REMOVE_DEV));
    }

    DeviceType type = (*iter)->GetType();
    ioDispatcher->RemoveDeviceForReactor(dev);
    ioDispatcher->RemoveDeviceForIOWorker(dev);
    devices.erase(iter);

    UnvmeSsdSharedPtr ssd = nullptr;
    if (type == DeviceType::SSD)
    {
        ssd = dynamic_pointer_cast<UnvmeSsd>(dev);
    }
    if (type == DeviceType::SSD && ssd != nullptr)
    {
        POS_TRACE_WARN(EID(DEVICEMGR_REMOVE_DEV), "SSD {}({}) is hot-removed", dev->GetName(), dev->GetSN());
        ssd->SetHotDetached();
        UnvmeDrvSingleton::Instance()->GetDaemon()->Resume();
    }

    POS_TRACE_WARN(EID(DEVICEMGR_REMOVE_DEV),
        "device removed successfully name:{}, type:{}", dev->GetName(), type);
    dev = nullptr;

    return 0;
}

void
DeviceManager::_Rescan()
{
    // URAM RESCAN
    POS_TRACE_INFO(EID(DEVICEMGR_RESCAN), "ReScan begin");
    size_t i;
    for (i = 0; i < drivers.size(); i++)
    {
        if (drivers[i]->GetName() == "UramDrv")
        {
            break;
        }
    }

    if (i >= drivers.size())
    {
        // NO URAM_DRV
        POS_TRACE_INFO(EID(DEVICEMGR_RESCAN),
            "ReScan done, dev cnt: {}", devices.size());
        return;
    }

    POS_TRACE_DEBUG(EID(DEVICEMGR_RESCAN),
        "scanning {}, {}", i, drivers[i]->GetName());
    vector<UblockSharedPtr> _devs;
    drivers[i]->ScanDevs(&_devs);

    std::vector<UblockSharedPtr>::iterator it;
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
            _devs[j] = nullptr;
        }
    }

    POS_TRACE_INFO(EID(DEVICEMGR_RESCAN),
        "ReScan done, dev cnt: {}", devices.size());
}

UblockSharedPtr
DeviceManager::GetDev(DeviceIdentifier& devID)
{
    auto it = find_if(devices.begin(), devices.end(), devID.GetPredicate());
    if (it != devices.end())
    {
        POS_TRACE_DEBUG(EID(DEVICEMGR_GETDEV),
            "Device Found: {}", devID.val);
        return (*it);
    }

    POS_TRACE_INFO(EID(DEVICEMGR_GETDEV),
        "Device Not Found: {}", devID.val);
    return nullptr;
}

vector<UblockSharedPtr>
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

    return devs;
}

struct spdk_nvme_ctrlr*
DeviceManager::GetNvmeCtrlr(std::string& deviceName)
{
    std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
    for (UblockSharedPtr dev : devices)
    {
        DeviceType deviceType = dev->GetType();
        if (DeviceType::SSD == deviceType &&
            (0 == static_cast<string>(dev->GetName()).compare(deviceName)))
        {
            UnvmeSsdSharedPtr unvmeSsd = static_pointer_cast<UnvmeSsd>(dev);
            return spdkNvmeCaller->SpdkNvmeNsGetCtrlr(unvmeSsd->GetNs());
        }
    }

    return nullptr;
}

void
DeviceManager::AttachDevice(UblockSharedPtr dev)
{
    bool isNew = false;
    {
        std::lock_guard<std::recursive_mutex> guard(deviceManagerMutex);
        if (_CheckDuplication(dev) == true)
        {
            _PrepareDevice(dev);
            devices.push_back(dev);
            isNew = true;
        }
    }
    if (isNew == true)
    {
        deviceEvent->DeviceAttached(dev);
        POS_TRACE_TRACE(EID(POS_TRACE_DEVICE_ATTACHED),
            "device_name:{}, device_mn:{}, device_sn:{}",
            dev->GetName(), dev->GetMN(), dev->GetSN());
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
            UblockSharedPtr dev = GetDev(uid);
            if (dev == nullptr)
            {
                // Device Manager can call DetachDevice for already removed Ublock Device.
                // So, just resume the Daemon.
                UnvmeDrvSingleton::Instance()->GetDaemon()->Resume();
                POS_TRACE_WARN(EID(DEVICEMGR_DETACH),
                    "DetachDevice - unknown device or already detached: {}", uid.val);
                return static_cast<int>(EID(DEVICEMGR_DETACH));
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
DeviceManager::_DetachDeviceImpl(UblockSharedPtr dev)
{
    POS_TRACE_TRACE(EID(POS_TRACE_DEVICE_DETACHED),
        "device_name:{}, device_mn:{}, device_sn:{}", dev->GetName(), dev->GetMN(), dev->GetSN());
    if (dev->GetClass() == DeviceClass::ARRAY)
    {
        if (deviceEvent != nullptr)
        {
            if (deviceEvent->DeviceDetached(dev) < 0)
            {
                POS_TRACE_DEBUG(EID(DEVICEMGR_DETACH),
                "Failed to acquire lock for device detach");
                return LOCK_ACQUIRE_FAILED;
            }
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
    cpu_set_t targetCpuSet =
        affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
    ioDispatcher->AddIOWorker(targetCpuSet);
}

void
DeviceManager::_PrepareDevices(void)
{
    for (auto& iter : devices)
    {
        _PrepareDevice(iter);
    }
}

void
DeviceManager::_PrepareDevice(UblockSharedPtr dev)
{
    /*
        Device must be added to the reactor before the ioworker.
        Adding uram to IOworker, it has recovery behavior.
        Then ioworker issue IO to the uram and then reactor access to uram context
    */
    ioDispatcher->AddDeviceForReactor(dev);
    cpu_set_t ioWorkerCpuSet = affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
    ioDispatcher->AddDeviceForIOWorker(dev, ioWorkerCpuSet);
    dev->WrapupOpenDeviceSpecific();
}

} // namespace pos
