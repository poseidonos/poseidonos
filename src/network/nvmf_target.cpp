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

#include "src/network/nvmf_target.hpp"

#include <iostream>
#include <string>

#include "spdk/ibof_volume.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/network/nvmf_target_spdk.hpp"
#include "src/network/nvmf_volume_ibof.hpp"
#include "src/sys_event/volume_event_publisher.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/qos/qos_manager.h"

using namespace std;

extern struct spdk_nvmf_tgt* g_spdk_nvmf_tgt;
namespace pos
{
struct NvmfTargetCallbacks NvmfTarget::nvmfCallbacks;
const char* NvmfTarget::BDEV_NAME_PREFIX = "bdev";
std::atomic<int> NvmfTarget::attachedNsid;

NvmfTarget::NvmfTarget(void)
{
    InitNvmfCallbacks(&nvmfCallbacks);
}

NvmfTarget::~NvmfTarget(void)
{
}

bool
NvmfTarget::CreateIBoFBdev(const string& bdevName, uint32_t id,
    uint64_t volumeSizeInMb, uint32_t blockSize, bool volumeTypeInMem, const string& arrayName)
{
    uint64_t volumeSizeInByte = volumeSizeInMb * MB;
    struct spdk_bdev* bdev = spdk_bdev_create_ibof_disk(bdevName.c_str(), id, nullptr,
        volumeSizeInByte / blockSize, blockSize, volumeTypeInMem, arrayName.c_str());
    if (bdev == nullptr)
    {
        SPDK_ERRLOG("bdev %s does not exist\n", bdevName.c_str());
        return false;
    }
    struct EventContext* ctx = _CreateEventContext(nullptr, nullptr,
        strdup(bdevName.c_str()), nullptr);
    if (!ctx)
    {
        return false;
    }
    nvmfCallbacks.createIbofBdevDone(ctx, NvmfCallbackStatus::SUCCESS);
    return true;
}

bool
NvmfTarget::DeleteIBoFBdev(const string& bdevName)
{
    struct spdk_bdev* bdev = spdk_bdev_get_by_name(bdevName.c_str());
    if (bdev == nullptr)
    {
        SPDK_ERRLOG("bdev %s does not exist\n", bdevName.c_str());
        return false;
    }

    struct EventContext* ctx = _CreateEventContext(nullptr, nullptr,
        strdup(bdevName.c_str()), nullptr);
    if (ctx == nullptr)
    {
        return false;
    }
    spdk_bdev_delete_ibof_disk(bdev, nvmfCallbacks.deleteIbofBdevDone, ctx);
    return true;
}

void
NvmfTarget::_AttachDone(void* cbArg, int status)
{
    attachedNsid = status;
}

void
NvmfTarget::_TryAttachHandler(void* arg1, void* arg2)
{
    string subnqn = *static_cast<string*>(arg1);
    string bdevName = *static_cast<string*>(arg2);

    NvmfTarget nvmfTarget;
    int ret = nvmfTarget.AttachNamespace(subnqn, bdevName.c_str(), 0, _AttachDone, nullptr);
    if (ret == false)
    {
        SPDK_ERRLOG("failed to try attach namespace(bdev: %s) to %s\n", bdevName.c_str(), subnqn.c_str());
        attachedNsid = NvmfCallbackStatus::FAILED;
    }

    delete (static_cast<string*>(arg1));
    delete (static_cast<string*>(arg2));
}

bool
NvmfTarget::TryToAttachNamespace(const string& nqn, int volId, string& arrayName)
{
    int yetAttached = -1;
    attachedNsid = -1;

    string* bdevName = new string(GetBdevName(volId, arrayName));
    string* subnqn = new string(nqn);

    EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
        _TryAttachHandler, static_cast<void*>(subnqn), static_cast<void*>(bdevName));
    while (attachedNsid == yetAttached)
    {
        usleep(1);
    }
    if (attachedNsid == NvmfCallbackStatus::FAILED)
    {
        SPDK_ERRLOG("failed to try attach namespace(vol:%d) to %s\n", volId, nqn.c_str());
        return false;
    }
    return true;
}

bool
NvmfTarget::AttachNamespace(const string& nqn, const string& bdevName,
    IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    return AttachNamespace(nqn, bdevName, 0, callback, arg);
}

bool
NvmfTarget::AttachNamespace(const string& nqn, const string& bdevName,
    uint32_t nsid, IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    if (!IsTargetExist())
    {
        SPDK_ERRLOG("fail to attach namespace: target does not exist\n");
        return false;
    }
    struct spdk_nvmf_subsystem* subsystem = FindSubsystem(nqn);
    if (subsystem == nullptr)
    {
        return false;
    }

    struct EventContext* ctx = _CreateEventContext(callback, arg,
        strdup(bdevName.c_str()), spdk_sprintf_alloc("%u", nsid));
    if (ctx == nullptr)
    {
        return false;
    }
    _AttachNamespaceWithPause(subsystem, ctx);

    return true;
}

void
NvmfTarget::_AttachNamespaceWithPause(void* arg1, void* arg2)
{
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;

    int ret = spdk_nvmf_subsystem_pause(subsystem, nvmfCallbacks.attachNamespacePauseDone, (void*)arg2);
    if (ret != 0)
    {
        SPDK_NOTICELOG("failed to pause subsystem during attaching namespace : retrying \n");
        EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(), _AttachNamespaceWithPause, subsystem, arg2);
    }
}

struct spdk_nvmf_ns*
NvmfTarget::GetNamespace(
    struct spdk_nvmf_subsystem* subsystem, const string& bdevName)
{
    struct spdk_bdev* bdev = nullptr;
    struct spdk_bdev* targetBdev = spdk_bdev_get_by_name(bdevName.c_str());
    if (targetBdev == nullptr)
    {
        SPDK_ERRLOG("failed to get namespace : bdev(%s) does not exist\n", bdevName.c_str());
        return nullptr;
    }
    struct spdk_nvmf_ns* ns = spdk_nvmf_subsystem_get_first_ns(subsystem);
    while (ns != nullptr)
    {
        bdev = spdk_nvmf_ns_get_bdev(ns);
        if (bdev == targetBdev)
        {
            return ns;
        }
        ns = spdk_nvmf_subsystem_get_next_ns(subsystem, ns);
    }
    return nullptr;
}

bool
NvmfTarget::DetachNamespace(const string& nqn, uint32_t nsid,
    IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    if (!IsTargetExist())
    {
        SPDK_ERRLOG("fail to detach namespace: target does not exist\n");
        return false;
    }
    if (nqn.empty())
    {
        SPDK_ERRLOG("fail to detache namespace: nqn does not exist\n");
        return false;
    }
    struct spdk_nvmf_subsystem* subsystem = FindSubsystem(nqn);
    if (subsystem == nullptr)
    {
        return false;
    }

    bool nsidUndefined = (nsid == 0);
    if (nsidUndefined)
    {
        struct ibof_volume_info* vInfo = (struct ibof_volume_info*)arg;
        string bdevName = GetBdevName(vInfo->id, vInfo->array_name);
        struct spdk_nvmf_ns* ns = GetNamespace(subsystem, bdevName);
        if (ns == nullptr)
        {
            SPDK_ERRLOG("failed to detach namespace : could not get namespace\n");
            return false;
        }
        nsid = spdk_nvmf_ns_get_id(ns);
    }

    struct EventContext* ctx = _CreateEventContext(callback, arg,
        spdk_sprintf_alloc("%u", nsid), nullptr);
    if (ctx == nullptr)
    {
        return false;
    }
    _DetachNamespaceWithPause(subsystem, ctx);

    return true;
}

bool
NvmfTarget::CheckSubsystemExistance(void)
{
    struct spdk_nvmf_tgt* nvmf_tgt;
    struct spdk_nvmf_subsystem* subsystem;
    nvmf_tgt = spdk_nvmf_get_tgt("nvmf_tgt");
    subsystem = spdk_nvmf_subsystem_get_first(nvmf_tgt);
    while (subsystem != NULL)
    {
        if (spdk_nvmf_subsystem_get_type(subsystem) == SPDK_NVMF_SUBTYPE_NVME)
        {
            return true;
        }
        subsystem = spdk_nvmf_subsystem_get_next(subsystem);
    }
    return false;
}

void
NvmfTarget::_DetachNamespaceWithPause(void* arg1, void* arg2)
{
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;

    int ret = spdk_nvmf_subsystem_pause(subsystem,
        nvmfCallbacks.detachNamespacePauseDone, (void*)arg2);
    if (ret != 0)
    {
        SPDK_NOTICELOG("failed to pause subsystem during detaching namespace : retrying \n");
        EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(), _DetachNamespaceWithPause, subsystem, arg2);
    }
}

bool
NvmfTarget::DetachNamespaceAll(const string& nqn,
    IBoFNvmfEventDoneCallback_t callback, void* arg)
{
    if (!IsTargetExist())
    {
        SPDK_ERRLOG("fail to detach namespace: target does not exist\n");
        return false;
    }

    struct spdk_nvmf_subsystem* subsystem = FindSubsystem(nqn);
    if (subsystem == nullptr)
    {
        return false;
    }

    struct EventContext* ctx = _CreateEventContext(callback, arg, nullptr, nullptr);
    if (ctx == nullptr)
    {
        return false;
    }

    _DetachNamespaceAllWithPause(subsystem, ctx);

    return true;
}

void
NvmfTarget::_DetachNamespaceAllWithPause(void* arg1, void* arg2)
{
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;

    int ret = spdk_nvmf_subsystem_pause(subsystem,
        nvmfCallbacks.detachNamespaceAllPauseDone, (void*)arg2);

    if (ret != 0)
    {
        SPDK_NOTICELOG("failed to pause subsystem(%s) during detaching ns :retrying \n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
        EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetFirstReactor(),
            _DetachNamespaceAllWithPause, subsystem, arg2);
    }
}

uint32_t
NvmfTarget::GetSubsystemNsCnt(struct spdk_nvmf_subsystem* subsystem)
{
    uint32_t nsCnt = 0;
    struct spdk_nvmf_ns* ns = spdk_nvmf_subsystem_get_first_ns(subsystem);
    while (ns != nullptr)
    {
        nsCnt++;
        ns = spdk_nvmf_subsystem_get_next_ns(subsystem, ns);
    }
    return nsCnt;
}

struct spdk_nvmf_subsystem*
NvmfTarget::AllocateSubsystem(void)
{
    static uint32_t allowedNsCnt = nrVolumePerSubsystem;

    struct spdk_nvmf_subsystem* subsystem = spdk_nvmf_subsystem_get_first(g_spdk_nvmf_tgt);
    while (subsystem != nullptr)
    {
        if (spdk_nvmf_subsystem_get_type(subsystem) == SPDK_NVMF_SUBTYPE_NVME)
        {
            uint32_t nsCnt = GetSubsystemNsCnt(subsystem);
            if (nsCnt < allowedNsCnt)
            {
                return subsystem;
            }
        }
        struct spdk_nvmf_subsystem* nextSubsystem = spdk_nvmf_subsystem_get_next(subsystem);
        if (nextSubsystem == nullptr)
        {
            nextSubsystem = spdk_nvmf_subsystem_get_first(g_spdk_nvmf_tgt);
            if (!nextSubsystem || spdk_nvmf_subsystem_get_next(nextSubsystem) == nullptr)
            {
                SPDK_ERRLOG("failed to allocate subsystem : next subsystem does not exist\n");
                return nullptr;
            }
            allowedNsCnt += nrVolumePerSubsystem;
        }
        subsystem = nextSubsystem;
    }
    return nullptr;
}

string
NvmfTarget::GetBdevName(uint32_t id, string arrayName)
{
    return BDEV_NAME_PREFIX + to_string(id) + arrayName;
}

string
NvmfTarget::GetVolumeNqn(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_nqn(subsystem);
}

uint32_t
NvmfTarget::GetVolumeNqnId(const string& subnqn)
{
    spdk_nvmf_subsystem* subsystem = FindSubsystem(subnqn);
    if (nullptr == subsystem)
    {
        return -1;
    }
    return spdk_nvmf_subsystem_get_id(subsystem);
}

void
NvmfTarget::QosEnableDone(void* cbArg, int status)
{
}

void
NvmfTarget::SetVolumeQos(const string& bdevName, uint64_t maxIops, uint64_t maxBw)
{
    if (true == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        return;
    }
    uint64_t limits[SPDK_BDEV_QOS_NUM_RATE_LIMIT_TYPES];
    struct spdk_bdev* bdev = spdk_bdev_get_by_name(bdevName.c_str());
    if (bdev == nullptr)
    {
        SPDK_ERRLOG("Could not find the bdev: %s\n", bdevName.c_str());
        spdk_app_stop(-1);
        return;
    }

    limits[SPDK_BDEV_QOS_RW_IOPS_RATE_LIMIT] = maxIops;
    limits[SPDK_BDEV_QOS_RW_BPS_RATE_LIMIT] = maxBw;
    limits[SPDK_BDEV_QOS_R_BPS_RATE_LIMIT] = 0;
    limits[SPDK_BDEV_QOS_W_BPS_RATE_LIMIT] = 0;
    spdk_bdev_set_qos_rate_limits(bdev, limits, QosEnableDone, nullptr);
}

bool
NvmfTarget::IsTargetExist(void)
{
    if (g_spdk_nvmf_tgt == nullptr)
    {
        return false;
    }
    return true;
}

spdk_nvmf_subsystem*
NvmfTarget::FindSubsystem(const string& nqn)
{
    spdk_nvmf_subsystem* subsystem = spdk_nvmf_tgt_find_subsystem(g_spdk_nvmf_tgt, nqn.c_str());
    if (subsystem == nullptr)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn.c_str());
        return nullptr;
    }
    return subsystem;
}

struct EventContext*
NvmfTarget::_CreateEventContext(IBoFNvmfEventDoneCallback_t callback,
    void* userArg, void* eventArg1, void* eventArg2)
{
    struct EventContext* ctx = AllocEventContext(callback, userArg);
    if (ctx == nullptr)
    {
        SPDK_ERRLOG("fail to allocEventContext\n");
        return nullptr;
    }
    ctx->eventArg1 = eventArg1;
    ctx->eventArg2 = eventArg2;
    return ctx;
}

vector<string>
NvmfTarget::GetHostNqn(string subnqn)
{
    vector<string> hostNqns;
    struct spdk_nvmf_subsystem* subsystem =
        spdk_nvmf_tgt_find_subsystem(g_spdk_nvmf_tgt, subnqn.c_str());
    struct spdk_nvmf_ctrlr* ctrlr = spdk_nvmf_subsystem_get_first_ctrlr(subsystem);
    while (ctrlr != NULL)
    {
        string hostNqn = spdk_nvmf_subsystem_get_ctrlr_hostnqn(ctrlr);
        if (!hostNqn.empty())
        {
            hostNqns.push_back(hostNqn);
        }
        ctrlr = spdk_nvmf_subsystem_get_next_ctrlr(subsystem, ctrlr);
    }
    return hostNqns;
}

bool
NvmfTarget::CheckVolumeAttached(int volId, string arrayName)
{
    string bdevName = GetBdevName(volId, arrayName);
    const char* nqn = get_attached_subsystem_nqn(bdevName.c_str());
    if (nqn == nullptr)
    {
        return false;
    }
    string subnqn(nqn);
    if (subnqn.empty() == true)
    {
        return false;
    }
    struct spdk_nvmf_subsystem* subsystem = FindSubsystem(subnqn);
    struct spdk_nvmf_ns* ns = GetNamespace(subsystem, bdevName);
    if (ns != nullptr)
    {
        return true;
    }
    return false;
}
} // namespace pos
