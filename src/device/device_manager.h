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

#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include <atomic>
#include <future>
#include <string>
#include <utility>
#include <vector>

#include "device_identifier.h"
#include "i_device_event.h"
#include "spdk/nvme.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/base/device_driver.h"
#include "src/device/device_monitor.h"
#include "src/include/memory.h"
#include "src/lib/singleton.h"
#include "src/device/i_dev_info.h"
#include "src/spdk_wrapper/caller/spdk_nvme_caller.h"

using namespace std;

namespace pos
{
class AffinityManager;
class DeviceProperty;
class EventScheduler;
class UBlockDevice;
class IIODispatcher;

using DeviceIterFunc = function<void(UblockSharedPtr, void*)>;

class DeviceManager
: public IDevInfo
{
public:
    DeviceManager(
        AffinityManager* affinityManager = AffinityManagerSingleton::Instance(),
        SpdkNvmeCaller* spdkNvmeCaller = new SpdkNvmeCaller());

    virtual ~DeviceManager(void);

    virtual void Initialize(IIODispatcher* ioDispatcherInterface);

    virtual void ScanDevs(void);
    virtual UblockSharedPtr GetDev(DeviceIdentifier& devID);
    virtual vector<UblockSharedPtr> GetDevs(void);
    virtual vector<DeviceProperty> ListDevs(void);
    virtual void AttachDevice(UblockSharedPtr dev);
    virtual int DetachDevice(DevUid sn);
    virtual int RemoveDevice(UblockSharedPtr dev);
    virtual struct spdk_nvme_ctrlr* GetNvmeCtrlr(std::string& deviceName);

    virtual int IterateDevicesAndDoFunc(DeviceIterFunc func, void* ctx);
    virtual void SetDeviceEventCallback(IDeviceEvent* event);

private:
    void _InitDriver();
    void _InitMonitor();
    void _InitScan();
    void _Rescan();
    void _ClearDevices();
    void _ClearMonitors();
    int _DetachDeviceImpl(UblockSharedPtr dev);
    void _DetachDone(string devName);
    void _PrepareIOWorker(void);
    void _PrepareDevices(void);
    void _PrepareDevice(UblockSharedPtr dev);
    bool _CheckDuplication(UblockSharedPtr dev);
    void _StartMonitoring(void);

    static const int LOCK_ACQUIRE_FAILED = -1;

    vector<UblockSharedPtr> devices;
    std::recursive_mutex deviceManagerMutex;
    vector<DeviceDriver*> drivers;
    vector<DeviceMonitor*> monitors;
    vector<future<void>> monitorFutures;

    AffinityManager* affinityManager;
    SpdkNvmeCaller* spdkNvmeCaller;
    IDeviceEvent* deviceEvent = nullptr;
    IIODispatcher* ioDispatcher = nullptr;
};

void DeviceDetachEventHandler(string sn);

using DeviceManagerSingleton = Singleton<DeviceManager>;

} // namespace pos
#endif // DEVICE_MANAGER_H_
