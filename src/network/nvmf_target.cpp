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

#include "src/network/nvmf_target.h"

#include <iostream>
#include <sstream>
#include <string>

#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/pos_event_id.hpp"
#include "src/lib/system_timeout_checker.h"
#include "src/logger/logger.h"
#include "src/network/nvmf_target_spdk.h"
#include "src/network/nvmf_volume_pos.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/sys_event/volume_event_publisher.h"

using namespace std;

extern struct spdk_nvmf_tgt* g_spdk_nvmf_tgt;
namespace pos
{
struct NvmfTargetCallbacks NvmfTarget::nvmfCallbacks;
const char* NvmfTarget::BDEV_NAME_PREFIX = "bdev_";
std::atomic<int> NvmfTarget::attachedNsid;

NvmfTarget::NvmfTarget(void)
: NvmfTarget(SpdkCallerSingleton::Instance(),
      QosManagerSingleton::Instance()->IsFeQosEnabled(),
      EventFrameworkApiSingleton::Instance())
{
}

NvmfTarget::NvmfTarget(SpdkCaller* spdkCaller, bool feQosEnable, EventFrameworkApi* eventFrameworkApi)
: spdkCaller(spdkCaller),
  feQosEnable(feQosEnable),
  eventFrameworkApi(eventFrameworkApi)
{
    InitNvmfCallbacks(&nvmfCallbacks);
}

NvmfTarget::~NvmfTarget(void)
{
}

bool
NvmfTarget::CreatePosBdev(const string& bdevName, const string& uuid, uint32_t id,
    uint64_t volumeSizeInMb, uint32_t blockSize, bool volumeTypeInMem, const string& arrayName, uint64_t arrayId)
{
    uint64_t volumeSizeInByte = volumeSizeInMb * MB;
    uint64_t numBlocks = volumeSizeInByte / blockSize;
    struct spdk_uuid* bdev_uuid = nullptr;

    // spdkCaller->SpdkUuidParse(bdev_uuid, (char *)&uuid);

    struct spdk_bdev* bdev = spdkCaller->SpdkBdevGetByName(bdevName.c_str());
    if (nullptr != bdev)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IONVMF_BDEV_ALREADY_EXIST;
        POS_TRACE_INFO(static_cast<int>(eventId), PosEventId::GetString(eventId), bdevName);
        return false;
    }

    bdev = spdkCaller->SpdkBdevCreatePosDisk(bdevName.c_str(), id, bdev_uuid,
        numBlocks, blockSize, volumeTypeInMem, arrayName.c_str(), arrayId);
    if (bdev == nullptr)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IONVMF_FAIL_TO_CREATE_POS_BDEV;
        POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), bdevName);
        return false;
    }
    struct EventContext* ctx = _CreateEventContext(nullptr, nullptr,
        strdup(bdevName.c_str()), nullptr);
    if (!ctx)
    {
        return false;
    }
    nvmfCallbacks.createPosBdevDone(ctx, NvmfCallbackStatus::SUCCESS);
    return true;
}

bool
NvmfTarget::DeletePosBdev(const string& bdevName)
{
    struct spdk_bdev* bdev = spdkCaller->SpdkBdevGetByName(bdevName.c_str());
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
    spdkCaller->SpdkBdevDeletePosDisk(bdev, nvmfCallbacks.deletePosBdevDone, ctx);
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

    int ret = _AttachNamespaceWithNsid(subnqn, bdevName.c_str(), 0, _AttachDone, nullptr);
    if (ret == false)
    {
        SPDK_ERRLOG("failed to try attach namespace(bdev: %s) to %s\n", bdevName.c_str(), subnqn.c_str());
        attachedNsid = NvmfCallbackStatus::FAILED;
    }

    delete (static_cast<string*>(arg1));
    delete (static_cast<string*>(arg2));
}

bool
NvmfTarget::TryToAttachNamespace(const string& nqn, int volId, string& arrayName, uint64_t time)
{
    int yetAttached = -1;
    attachedNsid = -1;

    string* bdevName = new string(GetBdevName(volId, arrayName));
    string* subnqn = new string(nqn);

    eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
        _TryAttachHandler, static_cast<void*>(subnqn), static_cast<void*>(bdevName));

    SystemTimeoutChecker timeChecker;
    timeChecker.SetTimeout(time);

    while (attachedNsid == yetAttached)
    {
        if (true == timeChecker.CheckTimeout())
        {
            attachedNsid = NvmfCallbackStatus::FAILED;
            break;
        }
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
    PosNvmfEventDoneCallback_t callback, void* arg)
{
    return _AttachNamespaceWithNsid(nqn, bdevName, 0, callback, arg);
}

bool
NvmfTarget::_AttachNamespaceWithNsid(const string& nqn, const string& bdevName,
    uint32_t nsid, PosNvmfEventDoneCallback_t callback, void* arg)
{
    if (!_IsTargetExist())
    {
        SPDK_ERRLOG("fail to attach namespace: target does not exist\n");
        return false;
    }
    struct spdk_nvmf_subsystem* subsystem =
        SpdkCallerSingleton::Instance()->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, nqn.c_str());
    if (subsystem == nullptr)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn.c_str());
        return false;
    }

    struct EventContext* ctx = _CreateEventContext(callback, arg,
        strdup(bdevName.c_str()), spdk_sprintf_alloc("%u", nsid));
    if (ctx == nullptr)
    {
        return false;
    }

    int ret = SpdkCallerSingleton::Instance()->SpdkNvmfSubsystemPause(subsystem, nvmfCallbacks.attachNamespacePauseDone, ctx);
    if (ret != 0)
    {
        _AttachNamespaceWithPause(subsystem, ctx);
    }

    return true;
}

void
NvmfTarget::_AttachNamespaceWithPause(void* arg1, void* arg2)
{
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;
    int ret = SpdkCallerSingleton::Instance()->SpdkNvmfSubsystemPause(subsystem, nvmfCallbacks.attachNamespacePauseDone, (void*)arg2);
    if (ret != 0)
    {
        SPDK_NOTICELOG("failed to pause subsystem during attaching namespace : retrying \n");
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(), _AttachNamespaceWithPause, subsystem, arg2);
    }
}

struct spdk_nvmf_ns*
NvmfTarget::GetNamespace(
    struct spdk_nvmf_subsystem* subsystem, const string& bdevName)
{
    struct spdk_bdev* bdev = nullptr;
    struct spdk_bdev* targetBdev = spdkCaller->SpdkBdevGetByName(bdevName.c_str());
    if (targetBdev == nullptr)
    {
        SPDK_ERRLOG("failed to get namespace : bdev(%s) does not exist\n", bdevName.c_str());
        return nullptr;
    }
    struct spdk_nvmf_ns* ns = spdkCaller->SpdkNvmfSubsystemGetFirstNs(subsystem);
    while (ns != nullptr)
    {
        bdev = spdkCaller->SpdkNvmfNsGetBdev(ns);
        if (bdev == targetBdev)
        {
            return ns;
        }
        ns = spdkCaller->SpdkNvmfSubsystemGetNextNs(subsystem, ns);
    }
    return nullptr;
}

bool
NvmfTarget::DetachNamespace(const string& nqn, uint32_t nsid,
    PosNvmfEventDoneCallback_t callback, void* arg)
{
    if (!_IsTargetExist())
    {
        SPDK_ERRLOG("fail to detach namespace: target does not exist\n");
        return false;
    }
    if (nqn.empty())
    {
        SPDK_ERRLOG("fail to detache namespace: nqn does not exist\n");
        return false;
    }
    struct spdk_nvmf_subsystem* subsystem =
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, nqn.c_str());
    if (subsystem == nullptr)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn.c_str());
        return false;
    }

    bool nsidUndefined = (nsid == 0);
    if (nsidUndefined)
    {
        struct pos_volume_info* vInfo = (struct pos_volume_info*)arg;
        string bdevName = GetBdevName(vInfo->id, vInfo->array_name);
        struct spdk_nvmf_ns* ns = GetNamespace(subsystem, bdevName);
        if (ns == nullptr)
        {
            SPDK_ERRLOG("failed to detach namespace : could not get namespace\n");
            return false;
        }
        nsid = spdkCaller->SpdkNvmfNsGetId(ns);
    }

    struct EventContext* ctx = _CreateEventContext(callback, arg,
        spdk_sprintf_alloc("%u", nsid), nullptr);
    if (ctx == nullptr)
    {
        return false;
    }

    int ret = spdkCaller->SpdkNvmfSubsystemPause(subsystem, nvmfCallbacks.detachNamespacePauseDone, ctx);
    if (ret != 0)
    {
        _DetachNamespaceWithPause(subsystem, ctx);
    }

    return true;
}

bool
NvmfTarget::CheckSubsystemExistance(void)
{
    struct spdk_nvmf_tgt* nvmf_tgt;
    struct spdk_nvmf_subsystem* subsystem;
    nvmf_tgt = spdkCaller->SpdkNvmfGetTgt("nvmf_tgt");
    subsystem = spdkCaller->SpdkNvmfSubsystemGetFirst(nvmf_tgt);
    while (subsystem != NULL)
    {
        if (spdkCaller->SpdkNvmfSubsystemGetType(subsystem) == SPDK_NVMF_SUBTYPE_NVME)
        {
            return true;
        }
        subsystem = spdkCaller->SpdkNvmfSubsystemGetNext(subsystem);
    }
    return false;
}

void
NvmfTarget::_DetachNamespaceWithPause(void* arg1, void* arg2)
{
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;

    int ret = SpdkCallerSingleton::Instance()->SpdkNvmfSubsystemPause(subsystem,
        nvmfCallbacks.detachNamespacePauseDone, (void*)arg2);
    if (ret != 0)
    {
        SPDK_NOTICELOG("failed to pause subsystem during detaching namespace : retrying \n");
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(), _DetachNamespaceWithPause, subsystem, arg2);
    }
}

bool
NvmfTarget::DetachNamespaceAll(const string& nqn,
    PosNvmfEventDoneCallback_t callback, void* arg)
{
    if (!_IsTargetExist())
    {
        SPDK_ERRLOG("fail to detach namespace: target does not exist\n");
        return false;
    }

    struct spdk_nvmf_subsystem* subsystem =
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, nqn.c_str());
    if (subsystem == nullptr)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn.c_str());
        return false;
    }

    struct EventContext* ctx = _CreateEventContext(callback, arg, nullptr, nullptr);
    if (ctx == nullptr)
    {
        return false;
    }

    int ret = spdkCaller->SpdkNvmfSubsystemPause(subsystem, nvmfCallbacks.detachNamespaceAllPauseDone, ctx);
    if (ret != 0)
    {
        _DetachNamespaceAllWithPause(subsystem, ctx);
    }

    return true;
}

void
NvmfTarget::_DetachNamespaceAllWithPause(void* arg1, void* arg2)
{
    struct spdk_nvmf_subsystem* subsystem = (struct spdk_nvmf_subsystem*)arg1;

    int ret = SpdkCallerSingleton::Instance()->SpdkNvmfSubsystemPause(subsystem,
        nvmfCallbacks.detachNamespaceAllPauseDone, (void*)arg2);

    if (ret != 0)
    {
        SPDK_NOTICELOG("failed to pause subsystem(%s) during detaching ns :retrying \n",
            spdk_nvmf_subsystem_get_nqn(subsystem));
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
            _DetachNamespaceAllWithPause, subsystem, arg2);
    }
}

uint32_t
NvmfTarget::GetSubsystemNsCnt(struct spdk_nvmf_subsystem* subsystem)
{
    uint32_t nsCnt = 0;
    struct spdk_nvmf_ns* ns = spdkCaller->SpdkNvmfSubsystemGetFirstNs(subsystem);
    while (ns != nullptr)
    {
        nsCnt++;
        ns = spdkCaller->SpdkNvmfSubsystemGetNextNs(subsystem, ns);
    }
    return nsCnt;
}

struct spdk_nvmf_subsystem*
NvmfTarget::AllocateSubsystem(void)
{
    static uint32_t allowedNsCnt = nrVolumePerSubsystem;

    struct spdk_nvmf_subsystem* subsystem = spdkCaller->SpdkNvmfSubsystemGetFirst(g_spdk_nvmf_tgt);
    while (subsystem != nullptr)
    {
        if (spdkCaller->SpdkNvmfSubsystemGetType(subsystem) == SPDK_NVMF_SUBTYPE_NVME)
        {
            uint32_t nsCnt = GetSubsystemNsCnt(subsystem);
            if (nsCnt < allowedNsCnt)
            {
                return subsystem;
            }
        }
        struct spdk_nvmf_subsystem* nextSubsystem = spdkCaller->SpdkNvmfSubsystemGetNext(subsystem);
        if (nextSubsystem == nullptr)
        {
            nextSubsystem = spdkCaller->SpdkNvmfSubsystemGetFirst(g_spdk_nvmf_tgt);
            if (!nextSubsystem || spdkCaller->SpdkNvmfSubsystemGetNext(nextSubsystem) == nullptr)
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
    return BDEV_NAME_PREFIX + to_string(id) + "_" + arrayName;
}

string
NvmfTarget::GetVolumeNqn(struct spdk_nvmf_subsystem* subsystem)
{
    return spdkCaller->SpdkNvmfSubsystemGetNqn(subsystem);
}

int32_t
NvmfTarget::GetVolumeNqnId(const string& nqn)
{
    struct spdk_nvmf_subsystem* subsystem =
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, nqn.c_str());
    if (nullptr == subsystem)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn.c_str());
        return -1;
    }
    return spdkCaller->SpdkNvmfSubsystemGetId(subsystem);
}

struct spdk_nvmf_subsystem*
NvmfTarget::FindSubsystem(const string& subnqn)
{
    struct spdk_nvmf_subsystem* subsystem =
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, subnqn.c_str());
    if (nullptr == subsystem)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", subnqn.c_str());
    }
    return subsystem;
}

void
NvmfTarget::QosEnableDone(void* cbArg, int status)
{
}

void
NvmfTarget::SetVolumeQos(const string& bdevName, uint64_t maxIops, uint64_t maxBw)
{
    if (true == feQosEnable)
    {
        return;
    }
    uint64_t limits[SPDK_BDEV_QOS_NUM_RATE_LIMIT_TYPES];
    struct spdk_bdev* bdev = spdkCaller->SpdkBdevGetByName(bdevName.c_str());
    if (bdev == nullptr)
    {
        SPDK_ERRLOG("Could not find the bdev: %s\n", bdevName.c_str());
        return;
    }

    limits[SPDK_BDEV_QOS_RW_IOPS_RATE_LIMIT] = maxIops;
    limits[SPDK_BDEV_QOS_RW_BPS_RATE_LIMIT] = maxBw;
    limits[SPDK_BDEV_QOS_R_BPS_RATE_LIMIT] = 0;
    limits[SPDK_BDEV_QOS_W_BPS_RATE_LIMIT] = 0;
    spdkCaller->SpdkBdevSetQosRateLimits(bdev, limits, QosEnableDone, nullptr);
}

bool
NvmfTarget::_IsTargetExist(void)
{
    if (g_spdk_nvmf_tgt == nullptr)
    {
        return false;
    }
    return true;
}

struct EventContext*
NvmfTarget::_CreateEventContext(PosNvmfEventDoneCallback_t callback,
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
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, subnqn.c_str());
    struct spdk_nvmf_ctrlr* ctrlr = spdkCaller->SpdkNvmfSubsystemGetFirstCtrlr(subsystem);
    while (ctrlr != nullptr)
    {
        string hostNqn = spdkCaller->SpdkNvmfSubsystemGetCtrlrHostnqn(ctrlr);
        if (!hostNqn.empty())
        {
            hostNqns.push_back(hostNqn);
        }
        ctrlr = spdkCaller->SpdkNvmfSubsystemGetNextCtrlr(subsystem, ctrlr);
    }
    return hostNqns;
}

bool
NvmfTarget::CheckVolumeAttached(int volId, string arrayName)
{
    string bdevName = GetBdevName(volId, arrayName);
    const char* nqn = spdkCaller->SpdkGetAttachedSubsystemNqn(bdevName.c_str());
    if (nqn == nullptr)
    {
        return false;
    }
    string subnqn(nqn);
    if (subnqn.empty() == true)
    {
        return false;
    }
    struct spdk_nvmf_subsystem* subsystem =
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, nqn);
    if (nullptr == subsystem)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", nqn);
        return false;
    }
    struct spdk_nvmf_ns* ns = GetNamespace(subsystem, bdevName);
    if (ns != nullptr)
    {
        return true;
    }
    return false;
}

vector<pair<int, string>>
NvmfTarget::GetAttachedVolumeList(string& subnqn)
{
    vector<pair<int, string>> volList;
    struct spdk_nvmf_subsystem* subsystem =
        spdkCaller->SpdkNvmfTgtFindSubsystem(g_spdk_nvmf_tgt, subnqn.c_str());
    if (nullptr == subsystem)
    {
        SPDK_ERRLOG("fail to find subsystem(NQN=%s): it does not exist\n", subnqn.c_str());
        return volList;
    }
    struct spdk_nvmf_ns* ns = spdkCaller->SpdkNvmfSubsystemGetFirstNs(subsystem);

    while (ns != nullptr)
    {
        struct spdk_bdev* bdev = spdkCaller->SpdkNvmfNsGetBdev(ns);
        string bdevName = spdkCaller->SpdkBdevGetName(bdev);
        size_t volIdIdx = bdevName.find("_");
        if (volIdIdx == string::npos)
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::IONVMF_FAIL_TO_FIND_VOLID;
            POS_TRACE_WARN(static_cast<int>(eventId), PosEventId::GetString(eventId));
            ns = spdkCaller->SpdkNvmfSubsystemGetNextNs(subsystem, ns);
            continue;
        }
        size_t arrayNameIdx = bdevName.find("_", volIdIdx + 1);
        if (arrayNameIdx == string::npos)
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::IONVMF_FAIL_TO_FIND_ARRAYNAME;
            POS_TRACE_WARN(static_cast<int>(eventId), PosEventId::GetString(eventId));
            ns = spdkCaller->SpdkNvmfSubsystemGetNextNs(subsystem, ns);
            continue;
        }
        arrayNameIdx += 1;
        volIdIdx += 1;
        int volId = stoi(bdevName.substr(volIdIdx, arrayNameIdx - volIdIdx - 1));
        string arrayName = bdevName.substr(arrayNameIdx, bdevName.length() - arrayNameIdx);
        volList.push_back(make_pair(volId, arrayName));

        ns = spdkCaller->SpdkNvmfSubsystemGetNextNs(subsystem, ns);
    }
    return volList;
}

string
NvmfTarget::GetPosBdevUuid(uint32_t id, string arrayName)
{
    char uuidStr[SPDK_UUID_STRING_LEN];

    string bdevName = GetBdevName(id, arrayName);
    struct spdk_bdev* bdev = spdkCaller->SpdkBdevGetByName(bdevName.c_str());
    if (bdev == nullptr)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IONVMF_BDEV_DOES_NOT_EXIST;
        POS_TRACE_WARN(static_cast<int>(eventId), PosEventId::GetString(eventId), bdevName);
        return "";
    }
    const struct spdk_uuid* uuid = spdkCaller->SpdkBdevGetUuid(bdev);
    if (uuid == nullptr)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IONVMF_BDEV_UUID_DOES_NOT_EXIST;
        POS_TRACE_WARN(static_cast<int>(eventId), PosEventId::GetString(eventId), bdevName);
        return "";
    }
    int ret = spdkCaller->SpdkUuidFmtLower(uuidStr, sizeof(uuidStr), uuid);
    if (ret != 0)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IONVMF_FAIL_TO_CONVERT_UUID_INTO_STRING;
        POS_TRACE_WARN(static_cast<int>(eventId), PosEventId::GetString(eventId), bdevName);
        return "";
    }
    return uuidStr;
}
} // namespace pos
