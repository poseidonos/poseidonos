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

#include "src/qos/qos_volume_manager.h"

#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "src/logger/logger.h"
#include "src/qos/io_queue.h"
#include "src/qos/parameter_queue.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/qos/rate_limit.h"
#include "src/qos/submission_adapter.h"
#include "src/spdk_wrapper/connection_management.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{

std::atomic<int64_t> QosVolumeManager::remainingVolumeBw[MAX_VOLUME_COUNT];
std::atomic<int64_t> QosVolumeManager::remainingVolumeIops[MAX_VOLUME_COUNT];
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::QosVolumeManager(QosContext* qosCtx, bool feQos)
: VolumeEvent("QosManager", ""),
  feQosEnabled(feQos),
  qosContext(qosCtx)
{
    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
        {
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, false);
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, true);
            pendingIO[reactor][volId] = 0;
        }
        previousDelay[reactor] = 0;
    }
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, "");
    try
    {
        bwIopsRateLimit = new BwIopsRateLimit;
        parameterQueue = new ParameterQueue;
        ioQueue = new IoQueue<pos_io*>;
    }
    catch (std::bad_alloc& ex)
    {
        assert(0);
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
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, "");
    delete bwIopsRateLimit;
    delete parameterQueue;
    delete ioQueue;
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
    std::vector<int>::iterator it;
    pthread_rwlock_wrlock(&nqnLock);
    it = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volId);
    // add volume id in map only if it is not already there to avoid douplicate entries.
    if (it == nqnVolumeMap[nqnId].end())
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

std::vector<int>
QosVolumeManager::GetVolumeFromActiveSubsystem(uint32_t nqnId, bool withLock)
{
    if (unlikely(withLock))
    {
        pthread_rwlock_rdlock(&nqnLock);
    }
    std::vector<int>& nqnMap = nqnVolumeMap[nqnId];
    if (unlikely(withLock))
    {
        pthread_rwlock_unlock(&nqnLock);
    }
    return nqnMap;
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
QosVolumeManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, pos_io* volIo)
{
    if (false == feQosEnabled)
    {
        return;
    }
    uint32_t reactorId = EventFrameworkApi::GetCurrentReactor();
    uint32_t volId = volIo->volume_id;
    uint64_t currentBw = 0;
    uint64_t currentIO = 0;
    currentBw = volumeQosParam[reactorId][volId].currentBW;
    currentIO = volumeQosParam[reactorId][volId].currentIOs;
    if ((pendingIO[reactorId][volId] == 0) &&
        (_GlobalRateLimit(reactorId, volId) == false))
    {
        currentBw = currentBw + volIo->length;
        currentIO++;
        aioSubmission->Do(volIo);
        _UpdateRateLimit(reactorId, volId, volIo->length);
        remainingVolumeBw[volId] -= volIo->length;
        remainingVolumeIops[volId] -= 1;
    }
    else
    {
        pendingIO[reactorId][volId]++;
        _EnqueueVolumeUbio(reactorId, volId, volIo);
        while (!IsExitQosSet())
        {
             if (_GlobalRateLimit(reactorId, volId) == true)
            {
                break;
            }
            pos_io* queuedVolumeIo = nullptr;
            queuedVolumeIo = _DequeueVolumeUbio(reactorId, volId);
            if (queuedVolumeIo == nullptr)
            {
                break;
            }
            currentBw = currentBw + queuedVolumeIo->length;
            currentIO++;
            pendingIO[reactorId][volId]--;
            aioSubmission->Do(queuedVolumeIo);
            _UpdateRateLimit(reactorId, volId, queuedVolumeIo->length);
            remainingVolumeBw[volId] -= queuedVolumeIo->length;
            remainingVolumeIops[volId] -= 1;
        }
    }
    volumeQosParam[reactorId][volId].currentBW = currentBw;
    volumeQosParam[reactorId][volId].currentIOs = currentIO;
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
QosVolumeManager::_EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, pos_io* io)
{
    ioQueue->EnqueueIo(reactorId, volId, io);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
pos_io*
QosVolumeManager::_DequeueVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    return ioQueue->DequeueIo(reactorId, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw)
{
    qos_vol_policy volumePolicy;
    volumePolicy.maxBw = maxbw;
    // update max iops here to display for qos list
    volumePolicy.maxIops = maxiops;
    volumePolicy.policyChange = true;
    volumePolicy.maxValueChanged = true;
    QosManagerSingleton::Instance()->UpdateVolumePolicy(volId, volumePolicy);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeCreated(std::string volName, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    _UpdateVolumeMaxQos(volID, maxiops, maxbw);
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName)
{
    qos_vol_policy volumePolicy;
    volumePolicy.policyChange = true;
    QosManagerSingleton::Instance()->UpdateVolumePolicy(volID, volumePolicy);
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    _UpdateVolumeMaxQos(volID, maxiops, maxbw);
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeUnmounted(std::string volName, int volID, std::string arrayName)
{
    if (false == feQosEnabled)
    {
        return true;
    }
    string bdevName = _GetBdevName(volID, arrayName);
    uint32_t nqnId = SpdkConnection::GetAttachedSubsystemId(volName.c_str());
    pthread_rwlock_wrlock(&nqnLock);
    std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volID);
    if (position != nqnVolumeMap[nqnId].end())
    {
        nqnVolumeMap[nqnId].erase(position);
    }
    pthread_rwlock_unlock(&nqnLock);
    _ClearVolumeParameters(volID);
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeLoaded(std::string volName, int id, uint64_t totalSize,
    uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
    uint64_t maxbw, std::string arrayName)
{
    qos_vol_policy volumePolicy = QosManagerSingleton::Instance()->GetVolumePolicy(volID);
    if ((volumePolicy.maxBw == maxbw) && (volumePolicy.maxIops == maxiops))
    {
        return true;
    }
    _UpdateVolumeMaxQos(volID, maxiops, maxbw);
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::VolumeDetached(vector<int> volList, std::string arrayName)
{
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
QosVolumeManager::ResetVolumeThrottling(int volId)
{
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(volId);
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
void
QosVolumeManager::_ResetRateLimit(uint32_t reactor, int volId, double offset)
{
    int64_t setBwLimit = GetVolumeLimit(reactor, volId, false);
    int64_t setIopsLimit = GetVolumeLimit(reactor, volId, true);
    bwIopsRateLimit->ResetRateLimit(reactor, volId, offset, setBwLimit, setIopsLimit);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeQosPoller(struct poller_structure* param, IbofIoSubmissionAdapter* aioSubmission)
{
    if (false == feQosEnabled)
    {
        return 0;
    }
    uint32_t retVal = 0;
    uint32_t reactor = param->id;
    uint64_t now = SpdkConnection::SpdkGetTicks();
    pos_io* queuedVolumeIo = nullptr;
    uint64_t currentBW = 0;
    uint64_t currentIO = 0;
    uint64_t next_tick = param->nextTimeStamp;
    if (now < next_tick)
    {
        return 0;
    }
    double offset = (double)(now - next_tick + previousDelay[reactor]) / param->qosTimeSlice;
    offset = offset + 1.0;
    volList[reactor].clear();
    pthread_rwlock_rdlock(&nqnLock);
    for (auto it = nqnVolumeMap.begin(); it != nqnVolumeMap.end(); it++)
    {
        uint32_t subsys = it->first;
        if (SpdkConnection::SpdkNvmfGetReactorSubsystemMapping(reactor, subsys) != M_INVALID_SUBSYSTEM)
        {
            volList[reactor][subsys] = GetVolumeFromActiveSubsystem(subsys, false);
        }
    }
    pthread_rwlock_unlock(&nqnLock);

    for (auto subsystem = volList[reactor].begin(); subsystem != volList[reactor].end(); subsystem++)
    {
        std::vector<int>& volumeList = volList[reactor][subsystem->first];

        for (uint32_t i = 0; i < volumeList.size(); i++)
        {
            int volId = volumeList[i];
            currentBW = 0;
            currentIO = 0;
            _ResetRateLimit(reactor, volId, offset);
            _EnqueueVolumeParameter(reactor, volId, offset);
            while (!IsExitQosSet())
            {
                if (_GlobalRateLimit(reactor, volId) == true)
                {
                    break;
                }
                queuedVolumeIo = _DequeueVolumeUbio(reactor, volId);
                if (queuedVolumeIo == nullptr)
                {
                    break;
                }
                currentBW = currentBW + queuedVolumeIo->length;
                currentIO++;
                pendingIO[reactor][volId]--;
                aioSubmission->Do(queuedVolumeIo);
                _UpdateRateLimit(reactor, volId, queuedVolumeIo->length);
                remainingVolumeBw[volId] -= queuedVolumeIo->length;
                remainingVolumeIops[volId] -= 1;
            }
            volumeQosParam[reactor][volId].currentBW = currentBW;
            volumeQosParam[reactor][volId].currentIOs = currentIO;
        }
    }
    qosContext->SetReactorProcessed(reactor, true);
    uint64_t after = SpdkConnection::SpdkGetTicks();
    previousDelay[reactor] = after - now;
    param->nextTimeStamp = after + param->qosTimeSlice / POLLING_FREQ_PER_QOS_SLICE;
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

    enqueueParameters = (!((currentBW == 0) && (pendingIO[reactor][volId] == 0)));
    // Condition (1) minimumPolicyInEffect || (0 != volPolicy.maxBw) || (0 != volPolicy.maxIops)
    // checks for some qos policy being present on the volume
    // Condition (2) "(!((currentBW == 0) && (pendingIO[reactor][volId] == 0)))" means its an active volume.
    // For any inactive volume the BW and well as pending IO count will be 0. Any other cases is active volume.
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
QosVolumeManager::_ClearVolumeParameters(uint32_t volId)
{
    parameterQueue->ClearParameters(volId);
}

} // namespace pos
