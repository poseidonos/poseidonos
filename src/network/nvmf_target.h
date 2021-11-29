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

#pragma once

#include <atomic>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "src/include/nvmf_const.h"
#include "src/lib/singleton.h"
#include "src/network/nvmf_target_spdk.h"
#include "src/spdk_wrapper/caller/spdk_caller.h"
#include "src/spdk_wrapper/caller/spdk_nvmf_caller.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/caller/spdk_bdev_caller.h"
using namespace std;
class EventFrameworkApi;

namespace pos
{
class ConfigManager;
enum NvmfCallbackStatus
{
    SUCCESS = 0,
    FAILED = -2,
    PARTIAL_FAILED = -1
};

class NvmfTarget
{
public:
    NvmfTarget(void);
    NvmfTarget(SpdkCaller* spdkCaller, bool feQosEnable,
        EventFrameworkApi* eventFrameworkApi, SpdkNvmfCaller* spdkNvmfCaller = nullptr, ConfigManager* configManager = nullptr);
    virtual ~NvmfTarget(void);

    virtual bool CreatePosBdev(const string& bdevName, const string uuid, uint32_t id, uint64_t volumeSizeInMb,
        uint32_t blockSize, bool volumeTypeInMem, const string& arrayName, uint64_t arrayId);
    virtual bool DeletePosBdev(const string& bdevName);
    virtual bool DeletePosBdevAll(string arrayName, uint64_t time = NS_DELETE_TIMEOUT);

    virtual bool DetachNamespace(const string& nqn, uint32_t nsid,
        PosNvmfEventDoneCallback_t cb, void* cbArg);
    virtual bool DetachNamespaceAll(const string& nqn, PosNvmfEventDoneCallback_t cb,
        void* cbArg);

    uint32_t GetSubsystemNsCnt(struct spdk_nvmf_subsystem* subsystem);
    struct spdk_nvmf_subsystem* AllocateSubsystem(string& arrayName, uint64_t arrayId);
    virtual struct spdk_nvmf_ns* GetNamespace(struct spdk_nvmf_subsystem* subsystem,
        const string& bdevName);

    virtual void SetVolumeQos(const string& bdevName, uint64_t maxIops, uint64_t maxBw);
    static void QosEnableDone(void* cbArg, int status);

    virtual string GetBdevName(uint32_t id, string arrayName);
    string GetVolumeNqn(struct spdk_nvmf_subsystem* subsystem);
    virtual int32_t GetVolumeNqnId(const string& subnqn);
    virtual spdk_nvmf_subsystem* FindSubsystem(const string& subnqn);
    vector<string> GetHostNqn(string subnqn);
    virtual bool TryToAttachNamespace(const string& nqn, int volId, string& arrayName, uint64_t time = NS_ATTACH_TIMEOUT);
    virtual bool CheckSubsystemExistance(void);
    virtual bool CheckVolumeAttached(int volId, string arrayName);
    vector<pair<int, string>> GetAttachedVolumeList(string& nqn);
    static bool AttachNamespace(const string& nqn, const string& bdevName,
        PosNvmfEventDoneCallback_t cb, void* cbArg);
    string GetPosBdevUuid(uint32_t id, string arrayName);
    virtual bool SetSubsystemArrayName(string& subnqn, string& arrayName);
    virtual string GetSubsystemArrayName(string& subnqn);

    virtual void RemoveSubsystemArrayName(string& subnqn);

protected:
    static struct NvmfTargetCallbacks nvmfCallbacks;
    static atomic<bool> deleteDone;
    static void _DeletePosBdevAllHandler(void* arg1);
    static void _DeletePosBdevAllHandler(void* arg1, SpdkCaller* spdkCaller,
        SpdkBdevCaller* spdkBdevCaller = new SpdkBdevCaller());

    static bool _AttachNamespaceWithNsid(const string& nqn, const string& bdevName, uint32_t nsid,
        PosNvmfEventDoneCallback_t cb, void* cbArg, SpdkNvmfCaller* spdkNvmfCaller = nullptr);
    static void _AttachNamespaceWithPause(void* arg1, void* arg2,
        EventFrameworkApi* eventFrameworkApi = nullptr, SpdkNvmfCaller* spdkNvmfCaller = nullptr);
    static void _DetachNamespaceWithPause(void* arg1, void* arg2,
        EventFrameworkApi* eventFrameworkApi = nullptr, SpdkNvmfCaller* spdkNvmfCaller = nullptr);
    static void _DetachNamespaceAllWithPause(void* arg1, void* arg2,
        EventFrameworkApi* eventFrameworkApi = nullptr, SpdkNvmfCaller* spdkNvmfCaller = nullptr);

private:
    uint32_t nrVolumePerSubsystem = 1;
    static const char* BDEV_NAME_PREFIX;
    static atomic<int> attachedNsid;
    static atomic<int> deletedBdev;
    SpdkCaller* spdkCaller;
    bool feQosEnable;
    EventFrameworkApi* eventFrameworkApi;
    SpdkNvmfCaller* spdkNvmfCaller;
    ConfigManager* configManager;
    map<string, string> subsystemToArrayName;

    bool _InitPosBdev(string bdevName);
    static struct EventContext* _CreateEventContext(PosNvmfEventDoneCallback_t callback,
        void* userArg, void* eventArg1, void* eventArg2);
    static bool _IsTargetExist(void);
    static void _AttachDone(void* cbArg, int status);
    static void _DeleteDone(void* cbArg, int status);
    static void _DetachNamespaceWithPause(void* arg1, void* arg2);
    static void _AttachNamespaceWithPause(void* arg1, void* arg2);
    static void _DetachNamespaceAllWithPause(void* arg1, void* arg2);
    static void _TryAttachHandler(void* arg1, void* arg2);
};

using NvmfTargetSingleton = Singleton<NvmfTarget>;

} // namespace pos
