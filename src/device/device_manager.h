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

#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include <atomic>
#include <future>
#include <string>
#include <utility>
#include <vector>

#include "device_identifier.h"
#include "spdk/nvme.h"
#include "src/device/device_driver.h"
#include "src/device/device_monitor.h"
#include "src/include/memory.h"
#include "src/lib/singleton.h"

using namespace std;

namespace ibofos
{
class AffinityManager;
class DeviceProperty;
class EventScheduler;
class UBlockDevice;

using DeviceIterFunc = function<void(UBlockDevice*, void*)>;

class DeviceManager
{
public:
    DeviceManager(void);
    ~DeviceManager();

    void ScanDevs(void);
    UBlockDevice* GetDev(DeviceIdentifier& devID);
    vector<UBlockDevice*> GetDevs();
    vector<DeviceProperty> ListDevs();
    void AttachDevice(UBlockDevice* dev);
    int DetachDevice(DevUid sn);
    int RemoveDevice(UBlockDevice* dev);
    struct spdk_nvme_ctrlr* GetNvmeCtrlr(std::string& deviceName);
    int PassThroughNvmeAdminCommand(std::string& deviceName,
        struct spdk_nvme_cmd* cmd, void* buffer, uint32_t bufferSizeInBytes);

    void StartMonitoring();
    void StopMonitoring();
    vector<pair<string, string>> MonitoringState();

    void HandleCompletedCommand(void);

    int IterateDevicesAndDoFunc(DeviceIterFunc func, void* ctx);

private:
    void _InitDriver();
    void _InitMonitor();
    void _InitScan();
    void _Rescan();
    void _ClearDevices();
    void _ClearMonitors();
    void _ClearWorkers();
    int _DetachDeviceImpl(UBlockDevice* dev);
    void _DetachDone(string devName);
    void _PrepareIOWorker(void);
    void _PrepareDevices(void);
    void _PrepareDevice(UBlockDevice* dev);
    void _PrepareMockDevice(UBlockDevice* dev);
    void _ReleaseDevice(UBlockDevice* dev);
    bool _CheckDuplication(UBlockDevice* dev);

    void _SetupThreadModel(void);
    void _WaitRegistration(void);
    void _RegisterThread(void);
    static void _RegisterToAllDevices(void* arg1, void* arg2);
    static void _RegisterToDevice(void* arg1, void* arg2);

    void _RegisterThread(UBlockDevice* dev);
    void _RegisterThreadToDevice(UBlockDevice* dev);

    static const uint32_t EVENT_THREAD_CORE_RATIO = 1;
    static const int NO_DEVICE_ERROR = -1;
    static const int LOCK_ACQUIRE_FAILED = -1;

    vector<UBlockDevice*> devices;
    std::recursive_mutex deviceManagerMutex;
    vector<DeviceDriver*> drivers;
    vector<DeviceMonitor*> monitors;
    vector<future<void>> monitorFutures;

    EventScheduler* eventScheduler;
    AffinityManager* affinityManager;
    std::atomic<bool> reactorRegistered;
};

using DeviceManagerSingleton = Singleton<DeviceManager>;

} // namespace ibofos
#endif // DEVICE_MANAGER_H_
