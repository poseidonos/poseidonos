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
#include "src/logger/logger.h"

#include <iostream>
#include <unordered_set>

namespace pos
{


std::atomic<int64_t> QosVolumeManager::globalBwThrottling(0x0);
std::atomic<int64_t> QosVolumeManager::globalIopsThrottling(0x0);
std::atomic<int64_t> QosVolumeManager::globalRemainingVolumeBw(0x0);
std::atomic<int64_t> QosVolumeManager::globalRemainingVolumeIops(0x0);
std::atomic<int64_t> QosVolumeManager::globalCurrentBw(0x0);
std::atomic<int64_t> QosVolumeManager::globalCurrentIops(0x0);

std::atomic<int64_t> QosVolumeManager::notThrottledVolumesThrottling;
std::atomic<int64_t> QosVolumeManager::remainingNotThrottledVolumesBw;
std::atomic<bool>  QosVolumeManager::isMinVolume[MAX_VOLUME_COUNT];

thread_local uint32_t myClock = 0, myClock2 = 0;
std::atomic<uint32_t> globalClock(0);
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
    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
        {
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, false);
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, true);
            pendingIO[volId] = 0;
        }
    }
    volumeEventPublisher->RegisterSubscriber(this, "", arrayId);
    bwIopsRateLimit = new BwIopsRateLimit;
    parameterQueue = new ParameterQueue;
    for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        remainingVolumeBw[volId] = 0;
        remainingVolumeIops[volId] = 0;
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
//    delete ioQueue;
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

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_UpdateRateLimit(uint32_t reactor, int volId, uint64_t size)
{
    bwIopsRateLimit->UpdateRateLimit(reactor, volId, size);
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
    uint32_t reactorId = eventFrameworkApi->GetCurrentReactor();
    uint32_t volId = volIo->GetVolumeId();
   // uint32_t blockSize = 0;

 //   blockSize = volIo->GetSize();

   /* if ((pendingIO[volId] == 0) &&
        (_RateLimit(reactorId, volId) == false) && (_GlobalRateLimit() == false))
    {
        aioSubmission->Do(volIo);
        remainingVolumeBw[volId] -= blockSize;
        globalRemainingVolumeBw -= blockSize;
        globalCurrentBw += blockSize;
        currentVolumeBw[volId] += blockSize;
        remainingVolumeIops[volId] -= 1;
        globalRemainingVolumeIops -= 1;
        globalCurrentIops += 1;
        currentVolumeIops[volId] += 1;
    }
    else
    {*/
        pendingIO[volId]++;
        _EnqueueVolumeUbio(reactorId, volId, volIo);
  /*      while (!IsExitQosSet())
        {
             if (_RateLimit(reactorId, volId) == true || _GlobalRateLimit() == true)
            {
                break;
            }
            _PollingAndSubmit(aioSubmission, volId);
            break;
        }*/
  //  }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_EnqueueParams(uint32_t reactor, uint32_t volId, bw_iops_parameter& volume_param)
{
    parameterQueue->EnqueueParameter(reactor, volId, volume_param);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bw_iops_parameter
QosVolumeManager::DequeueParams(uint32_t reactor, uint32_t volId)
{
    return parameterQueue->DequeueParameter(reactor, volId);
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
QosVolumeManager::_EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, VolumeIoSmartPtr io)
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
QosVolumeManager::_DequeueVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    std::lock_guard<std::mutex> lock(volumePendingIOLock[volId]);
    if(ioQueue[volId].empty() == true)
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
QosVolumeManager::_GlobalRateLimit(void)
{
    bool results = false;
   /* if ((globalRemainingVolumeBw < 0) || (globalRemainingVolumeIops < 0))
    {
        results = true;
    }*/
    return results;
}
bool
QosVolumeManager::_SpecialRateLimit(void)
{
    bool results = false;
    if (remainingNotThrottledVolumesBw < 0)
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
QosVolumeManager::_RateLimit(uint32_t reactor, int volId)
{
    bool results = false;
    if ((remainingVolumeBw[volId] < 0) || (remainingVolumeIops[volId] < 0))
    {
        results = true;
    }
    return results;
}

int _UpdateTotalVolumePerformance(const air::JSONdoc& data)
{
    auto& objs = data["PERF_ARR_VOL"]["objs"];
    uint64_t totalBw = 0, totalIops = 0;
    for (auto& obj_it : objs)
    {
        auto& obj = objs[obj_it.first];
   //     std::stringstream stream_it;
   //     stream_it << obj["index"];
    //    uint32_t index {0};
   //     stream_it >> index;

        std::stringstream stream_bw;
        stream_bw << obj["bw"];
        uint64_t bw {0};
        stream_bw >> bw;

        std::stringstream stream_iops;
        stream_iops << obj["iops"];
        uint32_t iops {0};
        stream_iops >> iops;

        std::stringstream stream_thread_name;
        stream_thread_name << obj["target_name"];
        std::string thread_name = "";
        stream_thread_name >> thread_name;

      //  uint32_t arrayId = (index & 0xFF00) >> 8;
      //  uint32_t volumeId = (index & 0x00FF);
        totalBw += bw;
        totalIops += iops;
    }
    totalBw /= PARAMETER_COLLECTION_INTERVAL;
    totalIops /= PARAMETER_COLLECTION_INTERVAL;
    float currentBw = QosVolumeManager::globalBwThrottling + 1;
    QosVolumeManager::globalBwThrottling = currentBw * 1.02;
    if (QosVolumeManager::globalBwThrottling > totalBw * 1.05)
    {
        QosVolumeManager::globalBwThrottling = currentBw * 0.9;
    }
    float currentIops = QosVolumeManager::globalIopsThrottling + 1;
    QosVolumeManager::globalIopsThrottling += currentIops * 1.02;
    if (QosVolumeManager::globalIopsThrottling > totalIops * 1.05)
    {
        QosVolumeManager::globalIopsThrottling = currentIops * 0.9;
    }
    if (QosVolumeManager::globalBwThrottling < 10)
    {
        QosVolumeManager::globalBwThrottling = 10;
    }
    if (QosVolumeManager::globalIopsThrottling < 10)
    {
         QosVolumeManager::globalIopsThrottling = 10;
    }
    POS_TRACE_INFO(9999, "previous Bw : {} Iops : {}, Throttling Bw : {}, Iops : {}, totalBw : {}, totalIops : {}",
        currentBw, currentIops, QosVolumeManager::globalBwThrottling, QosVolumeManager::globalIopsThrottling, totalBw, totalIops);
    return 0;
}


std::atomic<bool> updated;
void
QosVolumeManager::InitGlobalThrottling(void)
{
    updated = true;
}

void
QosVolumeManager::ResetGlobalThrottling(void)
{

  //  POS_TRACE_INFO(9999, " remaining Bw : {}, Iops : {}",
    //    globalRemainingVolumeBw, globalRemainingVolumeIops);

    int64_t bwUnit = 1024ULL * 1024ULL / PARAMETER_COLLECTION_INTERVAL; //1mb
    if (bwUnit < (float)globalBwThrottling * 0.02)
    {
        bwUnit = globalBwThrottling * 0.02;
    }
    int64_t iopsUnit = 1024ULL / 4 / PARAMETER_COLLECTION_INTERVAL; // 256 iops
    if (iopsUnit < (float)globalIopsThrottling * 0.02)
    {
        iopsUnit = globalIopsThrottling * 0.02;
    }
    if (globalRemainingVolumeBw > 0)
    {
        globalBwThrottling = globalBwThrottling - bwUnit;
    }
    else
    {
        globalBwThrottling = globalBwThrottling + bwUnit * 2;
    }
    
    if (globalRemainingVolumeIops > 0)
    {
       globalIopsThrottling = globalIopsThrottling - iopsUnit;
    }
    else
    {
        globalIopsThrottling = globalIopsThrottling + iopsUnit * 2;
    }
    if (globalBwThrottling < bwUnit)
    {
        globalBwThrottling = bwUnit;
    }
    if (globalIopsThrottling < iopsUnit)
    {
         globalIopsThrottling = iopsUnit;
    }
    if (globalRemainingVolumeBw < 0)
    {
        globalRemainingVolumeBw += globalBwThrottling;
    }
    else
    {
        globalRemainingVolumeBw = globalBwThrottling * 1;
    }

    if (globalRemainingVolumeIops < 0)
    {
        globalRemainingVolumeIops += globalIopsThrottling;
    }
    else
    {
        globalRemainingVolumeIops = globalIopsThrottling * 1;
    }
    if (updated == true)
    {
     //   globalBwThrottling = 1024ULL * 1024ULL / PARAMETER_COLLECTION_INTERVAL;
     //   globalIopsThrottling = 1024ULL / 4 / PARAMETER_COLLECTION_INTERVAL;
        POS_TRACE_INFO(9999, "Reset!!!!");
        updated = false;
    }
    uint64_t totalMinVolumeBw = 0;
    QosUserPolicy& qosUserPolicy = qosContextGlobal->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    std::vector<std::pair<uint32_t, uint32_t>> minVols = allVolUserPolicy.GetMinimumGuaranteeVolume();
    for (auto iter : minVols)
    {
        uint32_t arrayId = iter.first;
        uint32_t minVolId = iter.second;
        VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, minVolId);
        if (volumeUserPolicy->IsMinimumVolume() == false)
        {
            isMinVolume[minVolId] = false;
            continue;
        }
        totalMinVolumeBw += volumeUserPolicy->GetMinBandwidth();
        isMinVolume[minVolId] = true;
    }
    notThrottledVolumesThrottling = globalBwThrottling - totalMinVolumeBw;
    if (notThrottledVolumesThrottling < bwUnit)
    {
        notThrottledVolumesThrottling = bwUnit;
    }
    if (remainingNotThrottledVolumesBw < 0)
    {
        remainingNotThrottledVolumesBw += notThrottledVolumesThrottling;
    }
    else
    {
        remainingNotThrottledVolumesBw = notThrottledVolumesThrottling * 1;
    }
    globalCurrentBw = 0;
    globalCurrentIops = 0;
    globalClock = (globalClock + 1) % 100;
}

uint64_t
QosVolumeManager::GetTotalVolumeBandwidth(void)
{
    return globalRemainingVolumeBw;
}

uint64_t
QosVolumeManager::GetTotalVolumeIops(void)
{
    return globalRemainingVolumeIops;
}

void
QosVolumeManager::ResetVolumeThrottling(int volId, uint32_t arrayId)
{
    uint64_t userSetBwWeight = bwThrottling[volId];
    uint64_t userSetIops = iopsThrottling[volId];
    int64_t remainingBw = remainingVolumeBw[volId];
    int64_t remainingIops = remainingVolumeIops[volId];
    currentVolumeBw[volId] = 0;
    currentVolumeIops[volId] = 0;
    if (remainingBw > 0)
    {
        remainingVolumeBw[volId] = userSetBwWeight * MAX_THROTTLING_RATE;
    }
    else
    {
        remainingVolumeBw[volId] += userSetBwWeight * MAX_THROTTLING_RATE;
    }
    if (remainingIops > 0)
    {
        remainingVolumeIops[volId] = userSetIops * MAX_THROTTLING_RATE;
    }
    else
    {
        remainingVolumeIops[volId] += userSetIops * MAX_THROTTLING_RATE;
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


bool QosVolumeManager::_PollingAndSubmit(IbofIoSubmissionAdapter* aioSubmission, uint32_t volId)
{
    uint64_t blockSize = 0;
    VolumeIoSmartPtr queuedVolumeIo = nullptr;
    queuedVolumeIo = _DequeueVolumeUbio(0, volId);
    if (queuedVolumeIo.get() == nullptr)
    {
        return false;
    }
    blockSize = queuedVolumeIo->GetSize();
    pendingIO[volId]--;
    aioSubmission->Do(queuedVolumeIo);
    remainingVolumeBw[volId] -= blockSize;
    globalRemainingVolumeBw -= blockSize;
    currentVolumeBw[volId] += blockSize;
    globalCurrentBw += blockSize;
    remainingVolumeIops[volId] -= 1;
    globalRemainingVolumeIops -= 1;
    globalCurrentIops += 1;
    currentVolumeIops[volId] += 1;
    if (isMinVolume[volId] == false)
    {
        remainingNotThrottledVolumesBw -= blockSize;
    }
    return true;
}

int
QosVolumeManager::VolumeQosPoller(uint32_t reactor, IbofIoSubmissionAdapter* aioSubmission, double offset)
{
    uint32_t retVal = 0;

    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    std::vector<std::pair<uint32_t, uint32_t>> minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
    uint32_t notSubmittedCount = 0;
    while (!IsExitQosSet())
    {  
        bool submitted = false;
        if (_GlobalRateLimit() == true)
        {
            //goto next;
            break;
            //break;
        }

        if (_RateLimit(reactor, gvolId) == true)
        {
        //      notSubmittedCount++;
            // volId = (volId + 1) % MAX_VOLUME_COUNT;
            goto next;
            //break;
            //continue;
        }
        if (isMinVolume[gvolId] == false && _SpecialRateLimit() == true)
        {
            goto next;
        }
        submitted = _PollingAndSubmit(aioSubmission, gvolId);
        if (submitted == false)
        {
            goto next;
        }
        else
        {
            notSubmittedCount = 0;
            continue;
        }
next:
        notSubmittedCount++;
        gvolId = (gvolId + 1) % MAX_VOLUME_COUNT;
        if (notSubmittedCount >= MAX_VOLUME_COUNT)
        {
            break;
        }
        /*  if (submitted == false)
        {
            // volId = (volId + 1) % MAX_VOLUME_COUNT;
            notSubmittedCount++;
        }
        else
        {
            notSubmittedCount = 0;
        }
        if (notSubmittedCount >= MAX_VOLUME_COUNT)
        {
            // volId = (volId + 1) % MAX_VOLUME_COUNT;
            break;
        }   */
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
QosVolumeManager::SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t value, bool iops)
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
QosVolumeManager::GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops)
{
    return 0;
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
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::EnqueueVolumeParamsUt(uint32_t reactor, uint32_t volId)
{
    uint32_t offset = 1;
    _EnqueueVolumeParameter(reactor, volId, offset);
}

} // namespace pos
