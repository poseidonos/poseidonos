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

namespace pos
{
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
bool
QosVolumeManager::_RateLimit(uint32_t reactor, int volId)
{
    return bwIopsRateLimit->IsLimitExceeded(reactor, volId);
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
    uint64_t currentBw = 0;
    uint64_t currentIO = 0;
    uint32_t blockSize = 0;

    currentBw = volumeQosParam[reactorId][volId].currentBW;
    currentIO = volumeQosParam[reactorId][volId].currentIOs;
    blockSize = volIo->GetSize();

    if ((pendingIO[volId] == 0) &&
        (_GlobalRateLimit(reactorId, volId) == false))
    {
        currentBw = currentBw + volIo->GetSize();
        currentIO++;
        aioSubmission->Do(volIo);
        _UpdateRateLimit(reactorId, volId, volIo->GetSize());
        remainingVolumeBw[volId] -= volIo->GetSize();
        remainingVolumeIops[volId] -= 1;
    }
    else
    {
        pendingIO[volId]++;
        _EnqueueVolumeUbio(reactorId, volId, volIo);
        while (!IsExitQosSet())
        {
             if (_GlobalRateLimit(reactorId, volId) == true)
            {
                break;
            }
            VolumeIoSmartPtr queuedVolumeIo = nullptr;
            queuedVolumeIo = _DequeueVolumeUbio(reactorId, volId);
            if (queuedVolumeIo == nullptr)
            {
                break;
            }
            currentBw = currentBw + queuedVolumeIo->GetSize();
            currentIO++;
            pendingIO[volId]--;
            aioSubmission->Do(queuedVolumeIo);
            _UpdateRateLimit(reactorId, volId, queuedVolumeIo->GetSize());
            remainingVolumeBw[volId] -= queuedVolumeIo->GetSize();
            remainingVolumeIops[volId] -= 1;
        }
    }
    volumeQosParam[reactorId][volId].currentBW = currentBw;
    volumeQosParam[reactorId][volId].currentIOs = currentIO;
    volumeQosParam[reactorId][volId].blockSize = blockSize;
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
QosVolumeManager::_GlobalRateLimit(uint32_t reactor, int volId)
{
    bool results = false;
    if ((remainingVolumeBw[volId] < 0) || (remainingVolumeIops[volId] < 0))
    {
        results = true;
    }
    return results;
}

void
QosVolumeManager::ResetVolumeThrottling(int volId, uint32_t arrayId)
{
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, volId);
    if (volumeUserPolicy != nullptr)
    {
        uint64_t userSetBwWeight = volumeUserPolicy->GetMaxBandwidth();
        uint64_t userSetIops = volumeUserPolicy->GetMaxIops();
        int64_t remainingBw = remainingVolumeBw[volId];
        int64_t remainingIops = remainingVolumeIops[volId];
        if (remainingBw > 0)
        {
            remainingVolumeBw[volId] = userSetBwWeight * GLOBAL_THROTTLING;
        }
        else
        {
            remainingVolumeBw[volId] += userSetBwWeight * GLOBAL_THROTTLING;
        }
        if (remainingIops > 0)
        {
            remainingVolumeIops[volId] = userSetIops * GLOBAL_THROTTLING;
        }
        else
        {
            remainingVolumeIops[volId] += userSetIops * GLOBAL_THROTTLING;
        }
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
/*    int64_t setBwLimit = GetVolumeLimit(reactor, volId, false);
    int64_t setIopsLimit = GetVolumeLimit(reactor, volId, true);
    bwIopsRateLimit->ResetRateLimit(reactor, volId, offset, setBwLimit, setIopsLimit);*/
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeQosPoller(uint32_t reactor, IbofIoSubmissionAdapter* aioSubmission, double offset)
{
    uint32_t retVal = 0;
    VolumeIoSmartPtr queuedVolumeIo = nullptr;
    uint64_t currentBW = 0;
    uint64_t currentIO = 0;
    volList[reactor].clear();
    pthread_rwlock_rdlock(&nqnLock);
    for (auto it = nqnVolumeMap.begin(); it != nqnVolumeMap.end(); it++)
    {
        uint32_t subsys = it->first;
        if (spdkPosNvmfCaller->SpdkNvmfGetReactorSubsystemMapping(reactor, subsys) != INVALID_SUBSYSTEM)
        {
            volList[reactor][subsys] = GetVolumeFromActiveSubsystem(subsys, false);
        }
    }
    pthread_rwlock_unlock(&nqnLock);
  
    for (uint32_t i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        int volId = i;
        currentBW = 0;
        currentIO = 0;
        ResetRateLimit(reactor, volId, offset);
        //_EnqueueVolumeParameter(reactor, volId, offset);
        while (!IsExitQosSet())
        {
            if (_GlobalRateLimit(reactor, volId) == true)
            {
                break;
            }
            queuedVolumeIo = _DequeueVolumeUbio(reactor, volId);
            if (queuedVolumeIo.get() == nullptr)
            {
                break;
            }
            currentBW = currentBW + queuedVolumeIo->GetSize();
            currentIO++;
            pendingIO[volId]--;
            aioSubmission->Do(queuedVolumeIo);
            _UpdateRateLimit(reactor, volId, queuedVolumeIo->GetSize());
            remainingVolumeBw[volId] -= queuedVolumeIo->GetSize();
            remainingVolumeIops[volId] -= 1;
        }
        volumeQosParam[reactor][volId].currentBW = currentBW;
        volumeQosParam[reactor][volId].currentIOs = currentIO;
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
    uint64_t currentBW = volumeQosParam[reactor][volId].currentBW / offset;
    uint64_t currentIops = volumeQosParam[reactor][volId].currentIOs / offset;
    bool enqueueParameters = false;

    enqueueParameters = !((currentBW == 0) && (pendingIO[volId] == 0));
    // Condition "(!((currentBW == 0) && (pendingIO[reactor][volId] == 0)))" means its an active volume.
    // For any inactive volume the BW and well as pending IO count will be 0. Any other cases would be active volume.
    // BW (0), Pending (0) ==> Non Active Volume
    // BW (0), Pending (!0) ==> Active, as 0 throttling would have been applied, so IO's in pending queue
    // BW (!0), Pending (0) ==> Active, as IO's have been submitted but throttling value not reached
    // BW (!0), Pending (!0) ==> Active, IO's have been submitted and throttling also active
    if (enqueueParameters)
    {
        volumeQosParam[reactor][volId].valid = M_VALID_ENTRY;
        volumeQosParam[reactor][volId].currentBW = currentBW;
        volumeQosParam[reactor][volId].currentIOs = currentIops;
        _EnqueueParams(reactor, volId, volumeQosParam[reactor][volId]);
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
QosVolumeManager::SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t value, bool iops)
{
    if (true == iops)
    {
        volReactorIopsWeight[reactor][volId] = value;
    }
    else
    {
        volReactorWeight[reactor][volId] = value;
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
    if (true == iops)
    {
        return volReactorIopsWeight[reactor][volId];
    }
    else
    {
        return volReactorWeight[reactor][volId];
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
