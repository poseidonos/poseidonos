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

#include "Air.h"
#include "src/qos/qos_volume_manager.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "src/logger/logger.h"
#include "src/qos/io_queue.h"
#include "src/qos/parameter_queue.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/qos/rate_limit.h"
#include "src/qos/submission_adapter.h"
#include "src/qos/qos_context.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

namespace pos
{

std::atomic<int64_t> QosVolumeManager::globalBwThrottling(0x0);
std::atomic<int64_t> QosVolumeManager::globalIopsThrottling(0x0);
std::atomic<int64_t> QosVolumeManager::globalRemainingVolumeBw(0x0);
std::atomic<int64_t> QosVolumeManager::globalRemainingVolumeIops(0x0);

std::atomic<int64_t> QosVolumeManager::notThrottledVolumesThrottlingBw;
std::atomic<int64_t> QosVolumeManager::remainingNotThrottledVolumesBw;
std::atomic<int64_t> QosVolumeManager::notThrottledVolumesThrottlingIops;
std::atomic<int64_t> QosVolumeManager::remainingNotThrottledVolumesIops;

thread_local uint32_t gvolId = 0;
QosContext* qosContextGlobal = nullptr;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::QosVolumeManager(QosContext* qosCtx, bool feQos, uint32_t arrayIndex,
    QosArrayManager* qosArrayMgr, EventFrameworkApi* eventFrameworkApiArg,
    QosManager* qosManager, SpdkPosNvmfCaller* spdkPosNvmfCaller,
    SpdkPosVolumeCaller* spdkPosVolumeCaller, VolumeEventPublisher* volumeEventPublisher)
: VolumeEvent("QosManager", "", arrayIndex),
  eventFrameworkApi(eventFrameworkApiArg),
  feQosEnabled(feQos),
  qosContext(qosCtx),
  qosArrayManager(qosArrayMgr),
  qosManager(qosManager),
  spdkPosNvmfCaller(spdkPosNvmfCaller),
  spdkPosVolumeCaller(spdkPosVolumeCaller),
  volumeEventPublisher(volumeEventPublisher)
{
    qosContextGlobal = qosContext;
    uint32_t arrayId = arrayIndex;
    volumeEventPublisher->RegisterSubscriber(this, "", arrayId);
    bwIopsRateLimit = new BwIopsRateLimit;
    parameterQueue = new ParameterQueue;
    for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        SetVolumeLimit(volId, DEFAULT_MAX_BW_IOPS, false);
        SetVolumeLimit(volId, DEFAULT_MAX_BW_IOPS, true);
        pendingIO[volId] = 0;
        remainingVolumeBw[volId] = 0;
        remainingVolumeIops[volId] = 0;
        dynamicBwThrottling[volId] = 0;
        dynamicIopsThrottling[volId] = 0;
    }
    pthread_rwlock_init(&nqnLock, nullptr);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::~QosVolumeManager(void)
{
    volumeEventPublisher->RemoveSubscriber(this, "", arrayId);
    delete bwIopsRateLimit;
    delete parameterQueue;
    if (spdkPosNvmfCaller != nullptr)
    {
        delete spdkPosNvmfCaller;
    }
    if (spdkPosVolumeCaller != nullptr)
    {
        delete spdkPosVolumeCaller;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId)
{
    pthread_rwlock_wrlock(&nqnLock);
    if (std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volId) != nqnVolumeMap[nqnId].end())
    {
        pthread_rwlock_unlock(&nqnLock);
        return;
    }
    nqnVolumeMap[nqnId].push_back(volId);
    pthread_rwlock_unlock(&nqnLock);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId)
{
    std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volId);
    if (position != nqnVolumeMap[nqnId].end())
    {
        nqnVolumeMap[nqnId].erase(position);
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

std::vector<int>
QosVolumeManager::GetVolumeFromActiveSubsystem(uint32_t nqnId, bool withLock)
{
    std::vector<int> volumeList;
    if (withLock)
    {
        pthread_rwlock_rdlock(&nqnLock);
    }
    if (nqnVolumeMap.find(nqnId) != nqnVolumeMap.end())
    {
        volumeList = nqnVolumeMap[nqnId];
    }
    if (withLock)
    {
        pthread_rwlock_unlock(&nqnLock);
    }
    return volumeList;
}


bool
QosVolumeManager::_MinimumRateLimit(int volId)
{
    if (minVolumeBw[volId] != 0 && remainingDynamicVolumeBw[volId] < 0)
    {
        return true;
    }
    if (minVolumeIops[volId] != 0 && remainingDynamicVolumeIops[volId] < 0)
    {
        return true;
    }
    return false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, VolumeIoSmartPtr volIo)
{
    if (false == feQosEnabled)
    {
        return;
    }
    uint32_t volId = volIo->GetVolumeId();

    if (pendingIO[volId] == 0 && _GlobalRateLimit() == false && _RateLimit(volId) == false
        && _SpecialRateLimit(volId) == false && _MinimumRateLimit(volId) == false)
    {
        SubmitVolumeIoToAio(aioSubmission, volId, volIo);
        return;
    }

    EnqueueVolumeIo(volId, volIo);
    pendingIO[volId]++;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_ClearVolumeParameters(uint32_t volId)
{
    parameterQueue->ClearParameters(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::EnqueueVolumeIo(uint32_t volId, VolumeIoSmartPtr io)
{
    std::lock_guard<std::mutex> lock(volumePendingIOLock[volId]);
    ioQueue[volId].push(io);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeIoSmartPtr
QosVolumeManager::DequeueVolumeIo(uint32_t volId)
{
    std::lock_guard<std::mutex> lock(volumePendingIOLock[volId]);
    if (ioQueue[volId].empty() == true)
    {
        return nullptr;
    }
    VolumeIoSmartPtr volumeIo = ioQueue[volId].front();
    ioQueue[volId].pop();
    return volumeIo;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    qos_vol_policy volumePolicy;
    volumePolicy.maxBw = maxbw;
    // update max iops here to display for qos list
    volumePolicy.maxIops = maxiops;
    volumePolicy.policyChange = true;
    volumePolicy.maxValueChanged = true;
    qosArrayManager->UpdateVolumePolicy(volId, volumePolicy);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    _UpdateVolumeMaxQos(volEventBase->volId, volEventPerf->maxiops, volEventPerf->maxbw, volArrayInfo->arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    qos_vol_policy volumePolicy;
    volumePolicy.policyChange = true;
    qosArrayManager->UpdateVolumePolicy(volEventBase->volId, volumePolicy);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info* vInfo = new (struct pos_volume_info);
    _CopyVolumeInfo(vInfo->array_name, (volArrayInfo->arrayName).c_str(), (volArrayInfo->arrayName).size());
    vInfo->id = volEventBase->volId;
    vInfo->iops_limit = volEventPerf->maxiops;
    vInfo->bw_limit = volEventPerf->maxbw;
    qosContext->SetVolumeOperationDone(false);

    // enqueue in same reactor as NvmfVolumePos::VolumeMounted so that pos_disk structure is populated with correct values before qos tries to access it.
    eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
        _VolumeMountHandler, vInfo, this);
    volumeMap[vInfo->id] = true;
    while (qosContext->GetVolumeOperationDone() == false)
    {
        if (qosContext->GetVolumeOperationDone() == true)
        {
            break;
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_VolumeMountHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* volInfo = static_cast<pos_volume_info*>(arg1);
    QosVolumeManager* qosVolumeManager = static_cast<QosVolumeManager*>(arg2);
    qosVolumeManager->_InternalVolMountHandlerQos(volInfo);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_InternalVolMountHandlerQos(struct pos_volume_info* volMountInfo)
{
    if (volMountInfo)
    {
        string bdevName = _GetBdevName(volMountInfo->id, volMountInfo->array_name);
        uint32_t nqnId = 0;
        nqnId = spdkPosVolumeCaller->GetAttachedSubsystemId(bdevName.c_str());
        UpdateSubsystemToVolumeMap(nqnId, volMountInfo->id);
        _UpdateVolumeMaxQos(volMountInfo->id, volMountInfo->iops_limit, volMountInfo->bw_limit, volMountInfo->array_name);
        qosContext->SetVolumeOperationDone(true);
        delete (volMountInfo);
    }
}

bool
QosVolumeManager::_SpecialRateLimit(uint32_t volId)
{
    bool results = false;
    if (minVolumeBw[volId] == 0 && minVolumeIops[volId] == 0 && remainingNotThrottledVolumesBw < 0)
    {
        results = true;
    }
    return results;
}

void
QosVolumeManager::GetMountedVolumes(std::list<uint32_t>& volumeList)
{
    for (uint32_t index = 0; index < MAX_VOLUME_COUNT; index++)
    {
        if (volumeMap[index] == true)
        {
            volumeList.push_back(index);
        }
    }
}

bool
QosVolumeManager::_RateLimit(int volId)
{
    bool results = false;
    if ((remainingVolumeBw[volId] < 0) || (remainingVolumeIops[volId] < 0))
    {
        results = true;
    }
    return results;
}

bool
QosVolumeManager::_GlobalRateLimit(void)
{
    bool results = false;
    // Todo : We can turn on this code later after enough experiments
    // if ((globalRemainingVolumeBw < 0) || (globalRemainingVolumeIops < 0))
    // {
    //    results = true;
    // }
    return results;
}

int64_t
QosVolumeManager::_GetThrottlingChange(int64_t remainingValue, int64_t plusUpdateUnit, uint64_t minusUpdateUnit)
{
    if (remainingValue < 0)
    {
        return plusUpdateUnit;
    }
    return -minusUpdateUnit;
}

int64_t
QosVolumeManager::_ResetThrottlingCommon(int64_t remainingValue, uint64_t currentThrottlingValue)
{
    if (remainingValue < 0)
    {
        return currentThrottlingValue + remainingValue;
    }
    return currentThrottlingValue;
}

void
QosVolumeManager::ResetGlobalThrottling(void)
{
    int64_t bwUnit = BASIC_BW_UNIT;
    int64_t iopsUnit = BASIC_IOPS_UNIT;
    bwUnit = std::max(bwUnit, static_cast<int64_t>(UNIT_GLOBAL_RATE * globalBwThrottling));
    iopsUnit = std::max(iopsUnit, static_cast<int64_t>(UNIT_GLOBAL_RATE * globalIopsThrottling));
    globalBwThrottling += _GetThrottlingChange(globalRemainingVolumeBw, 4 * bwUnit, bwUnit);
    globalIopsThrottling += _GetThrottlingChange(globalRemainingVolumeIops, 4 * iopsUnit, iopsUnit);

    globalBwThrottling = std::max(globalBwThrottling.load(), bwUnit);
    globalIopsThrottling = std::max(globalIopsThrottling.load(), iopsUnit);

    globalRemainingVolumeBw = _ResetThrottlingCommon(globalRemainingVolumeBw, globalBwThrottling);
    globalRemainingVolumeIops = _ResetThrottlingCommon(globalRemainingVolumeIops, globalIopsThrottling);
}

uint64_t
QosVolumeManager::GetDynamicVolumeThrottling(uint32_t volId, bool iops)
{
    if (iops)
    {
        return dynamicIopsThrottling[volId];
    }
    return dynamicBwThrottling[volId];
}

uint64_t
QosVolumeManager::GetGlobalThrottling(bool iops)
{
    if (iops)
    {
        return globalIopsThrottling;
    }
    return globalBwThrottling;
}

void
QosVolumeManager::SetRemainingThrottling(uint64_t total, uint64_t minVolTotal, bool iops)
{
    int64_t bwUnit = BASIC_BW_UNIT;
    int64_t iopsUnit = BASIC_IOPS_UNIT;

    if (iops)
    {
        notThrottledVolumesThrottlingIops = (total - minVolTotal);
        notThrottledVolumesThrottlingIops = std::max(notThrottledVolumesThrottlingIops.load(), iopsUnit);
        remainingNotThrottledVolumesIops = _ResetThrottlingCommon(remainingNotThrottledVolumesIops, notThrottledVolumesThrottlingIops);
    }
    else
    {
        notThrottledVolumesThrottlingBw = (total - minVolTotal);
        notThrottledVolumesThrottlingBw = std::max(notThrottledVolumesThrottlingBw.load(), bwUnit);
        remainingNotThrottledVolumesBw = _ResetThrottlingCommon(remainingNotThrottledVolumesBw, notThrottledVolumesThrottlingBw);
    }
}

void
QosVolumeManager::ResetVolumeThrottling(int volId, uint32_t arrayId)
{
    uint64_t userSetBwWeight = bwThrottling[volId];
    uint64_t userSetIops = iopsThrottling[volId];
    int64_t bwUnit = BASIC_BW_UNIT;
    int64_t iopsUnit = BASIC_IOPS_UNIT;
    bwUnit = std::max(bwUnit, static_cast<int64_t>(dynamicBwThrottling[volId] * UNIT_VOLUME_RATE));
    iopsUnit = std::max(iopsUnit, static_cast<int64_t>(dynamicIopsThrottling[volId] * UNIT_VOLUME_RATE));

    dynamicBwThrottling[volId] += _GetThrottlingChange(remainingDynamicVolumeBw[volId] - 0.05 * dynamicBwThrottling[volId], MIN_GUARANTEED_INCREASE_COEFFICIENT * bwUnit, bwUnit);
    dynamicIopsThrottling[volId] += _GetThrottlingChange(remainingDynamicVolumeIops[volId] - 0.05 * dynamicIopsThrottling[volId] , MIN_GUARANTEED_INCREASE_COEFFICIENT * iopsUnit, iopsUnit);

    if (minVolumeBw[volId] != 0 && dynamicBwThrottling[volId] > minVolumeBw[volId] * MIN_GUARANTEED_THROTTLING_RATE)
    {
        dynamicBwThrottling[volId] = minVolumeBw[volId] * MIN_GUARANTEED_THROTTLING_RATE;
        remainingDynamicVolumeBw[volId] = 0;
    }
    if (minVolumeIops[volId] != 0 &&  dynamicIopsThrottling[volId] > minVolumeIops[volId] * MIN_GUARANTEED_THROTTLING_RATE)
    {
        dynamicIopsThrottling[volId] = minVolumeIops[volId] * MIN_GUARANTEED_THROTTLING_RATE;
        remainingDynamicVolumeIops[volId] = 0;
    }

    remainingDynamicVolumeBw[volId] = _ResetThrottlingCommon(remainingDynamicVolumeBw[volId], dynamicBwThrottling[volId]);
    remainingDynamicVolumeIops[volId] = _ResetThrottlingCommon(remainingDynamicVolumeIops[volId], dynamicIopsThrottling[volId]);

    remainingVolumeBw[volId] = _ResetThrottlingCommon(remainingVolumeBw[volId], userSetBwWeight * MAX_THROTTLING_RATE);
    remainingVolumeIops[volId] = _ResetThrottlingCommon(remainingVolumeIops[volId], userSetIops * MAX_THROTTLING_RATE);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    if (false == feQosEnabled)
    {
        return (int)POS_EVENT_ID::VOL_EVENT_OK;
    }
    struct pos_volume_info* vInfo = new (struct pos_volume_info);
    _CopyVolumeInfo(vInfo->array_name, (volArrayInfo->arrayName).c_str(), (volArrayInfo->arrayName).size());
    vInfo->id = volEventBase->volId;
    vInfo->iops_limit = 0;
    vInfo->bw_limit = 0;
    qosContext->SetVolumeOperationDone(false);
    volumeMap[vInfo->id] = false;
    eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
        _VolumeUnmountHandler, vInfo, this);

    while (qosContext->GetVolumeOperationDone() == false)
    {
        if (qosContext->GetVolumeOperationDone() == true)
        {
            break;
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_VolumeUnmountHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* volUnmountInfo = static_cast<struct pos_volume_info*>(arg1);
    QosVolumeManager* qosVolumeManager = static_cast<QosVolumeManager*>(arg2);
    qosVolumeManager->_InternalVolUnmountHandlerQos(volUnmountInfo);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_InternalVolUnmountHandlerQos(struct pos_volume_info* volUnmountInfo)
{
    if (volUnmountInfo)
    {
        string bdevName = _GetBdevName(volUnmountInfo->id, volUnmountInfo->array_name);
        uint32_t nqnId = 0;
        nqnId = spdkPosVolumeCaller->GetAttachedSubsystemId(bdevName.c_str());
        std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volUnmountInfo->id);
        if (position != nqnVolumeMap[nqnId].end())
        {
            nqnVolumeMap[nqnId].erase(position);
        }
        _ClearVolumeParameters(volUnmountInfo->id);
        qosContext->SetVolumeOperationDone(true);
        delete(volUnmountInfo);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    _UpdateVolumeMaxQos(volEventBase->volId, volEventPerf->maxiops, volEventPerf->maxbw, volArrayInfo->arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    qos_vol_policy volumePolicy = qosArrayManager->GetVolumePolicy(volEventBase->volId);
    if ((volumePolicy.maxBw == volEventPerf->maxbw) && (volumePolicy.maxIops == volEventPerf->maxiops))
    {
        return (int)POS_EVENT_ID::VOL_EVENT_OK;
    }
    std::string arrName = GetArrayName();
    if (0 == arrayName.compare(arrName))
    {
        _UpdateVolumeMaxQos(volEventBase->volId, volEventPerf->maxiops, volEventPerf->maxbw, arrayName);
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    for (auto volId : volList)
    {
        struct pos_volume_info* vInfo = new (struct pos_volume_info);
        _CopyVolumeInfo(vInfo->array_name, (volArrayInfo->arrayName).c_str(), (volArrayInfo->arrayName).size());
        vInfo->id = volId;
        vInfo->iops_limit = 0;
        vInfo->bw_limit = 0;
        qosContext->SetVolumeOperationDone(false);

        eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
            _VolumeDetachHandler, vInfo, this);

        while (qosContext->GetVolumeOperationDone() == false)
        {
            if (qosContext->GetVolumeOperationDone() == true)
            {
                break;
            }
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_VolumeDetachHandler(void* arg1, void* arg2)
{
    struct pos_volume_info* volDetachInfo = static_cast<struct pos_volume_info*>(arg1);
    QosVolumeManager* qosVolumeManager = static_cast<QosVolumeManager*>(arg2);
    qosVolumeManager->_InternalVolDetachHandlerQos(volDetachInfo);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_InternalVolDetachHandlerQos(struct pos_volume_info* volDetachInfo)
{
    if (volDetachInfo)
    {
        string bdevName = _GetBdevName(volDetachInfo->id, volDetachInfo->array_name);
        uint32_t nqnId = 0;
        nqnId = spdkPosVolumeCaller->GetAttachedSubsystemId(bdevName.c_str());
        std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volDetachInfo->id);
        if (position != nqnVolumeMap[nqnId].end())
        {
            nqnVolumeMap[nqnId].erase(position);
        }
        _ClearVolumeParameters(volDetachInfo->id);
        qosContext->SetVolumeOperationDone(true);
        delete(volDetachInfo);
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::ResetRateLimit(uint32_t reactor, int volId, double offset)
{
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void QosVolumeManager::SubmitVolumeIoToAio(IbofIoSubmissionAdapter* aioSubmission, uint32_t volId, VolumeIoSmartPtr volumeIo)
{
    if (!feQosEnabled)
    {
        return;
    }
    uint64_t blockSize = 0;
    blockSize = volumeIo->GetSize();
    aioSubmission->Do(volumeIo);
    remainingVolumeBw[volId] -= blockSize;
    globalRemainingVolumeBw -= blockSize;
    remainingVolumeIops[volId] -= 1;
    globalRemainingVolumeIops -= 1;
    remainingDynamicVolumeBw[volId] -= blockSize;
    remainingDynamicVolumeIops[volId] -= 1;

    if (minVolumeBw[volId] == 0)
    {
        remainingNotThrottledVolumesBw -= blockSize;
    }

    if (minVolumeIops[volId] == 0)
    {
        remainingNotThrottledVolumesIops -= 1;
    }
}


bool QosVolumeManager::_PollingAndSubmit(IbofIoSubmissionAdapter* aioSubmission, uint32_t volId)
{
    VolumeIoSmartPtr queuedVolumeIo = nullptr;
    if (pendingIO[volId] == 0)
    {
        return false;
    }
    queuedVolumeIo = DequeueVolumeIo(volId);
    if (queuedVolumeIo.get() == nullptr)
    {
        return false;
    }
    pendingIO[volId]--;
    SubmitVolumeIoToAio(aioSubmission, volId, queuedVolumeIo);
    return true;
}

int
QosVolumeManager::VolumeQosPoller(IbofIoSubmissionAdapter* aioSubmission, double offset)
{
    uint32_t retVal = 0;

    for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        while (!IsExitQosSet())
        {
            bool submitted = false;
            if (_GlobalRateLimit() == true)
            {
                break;
            }
            if (_RateLimit(volId) == true)
            {
                break;
            }
            if (_SpecialRateLimit(volId) == true)
            {
                break;
            }
            if (_MinimumRateLimit(volId) == true)
            {
                break;
            }
            submitted = _PollingAndSubmit(aioSubmission, volId);
            if (submitted == false)
            {
                break;
            }
        }
    }
    return retVal;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_EnqueueVolumeParameter(uint32_t reactor, uint32_t volId, double offset)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::SetVolumeLimit(uint32_t volId, int64_t value, bool iops)
{
    if (iops == true)
    {
        iopsThrottling[volId] = value;
    }
    else
    {
        bwThrottling[volId] = value;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosVolumeManager::GetVolumeLimit(uint32_t volId, bool iops)
{
    if (iops == true)
    {
        return iopsThrottling[volId];
    }
    else
    {
        return bwThrottling[volId];
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subSysVolMap)
{
    std::unique_lock<std::mutex> uniqueLock(subsysVolMapLock);
    subSysVolMap = nqnVolumeMap;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::SetArrayName(std::string name)
{
    arrayName = name;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::string
QosVolumeManager::GetArrayName(void)
{
    return arrayName;
}

string
QosVolumeManager::_GetBdevName(uint32_t volId, string arrayName)
{
    return BDEV_NAME_PREFIX + to_string(volId) + "_" + arrayName;
}


void
QosVolumeManager::SetMinimumVolume(uint32_t volId, uint64_t value, bool iops)
{
    if (iops)
    {
        minVolumeIops[volId] = value;
    }
    else
    {
        minVolumeBw[volId] = value;
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_CopyVolumeInfo(char* destInfo, const char* srcInfo, int len)
{
    strncpy(destInfo, srcInfo, len);
    destInfo[len] = '\0';
}

} // namespace pos
