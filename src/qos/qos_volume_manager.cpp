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
#include "src/qos/qos_manager.h"
#include "src/qos/rate_limit.h"
#include "src/qos/submission_adapter.h"
#include "src/spdk_wrapper/connection_management.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::QosVolumeManager(bool feQos)
: VolumeEvent("QosManager", ""),
  feQosEnabled(feQos)
{
    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
        {
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, false);
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, true);
            pendingIO[reactor][volId] = 0;
        }
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
    nqnVolumeMap[nqnId].push_back(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::vector<int>
QosVolumeManager::GetVolumeFromActiveSubsystem(uint32_t nqnId)
{
    return nqnVolumeMap[nqnId];
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
    if ((pendingIO[reactorId][volId] == 0) && (_RateLimit(reactorId, volId) == false))
    {
        currentBw = currentBw + volIo->length;
        currentIO++;
        aioSubmission->Do(volIo);
        _UpdateRateLimit(reactorId, volId, volIo->length);
    }
    else
    {
        pendingIO[reactorId][volId]++;
        _EnqueueVolumeUbio(reactorId, volId, volIo);
        while (1)
        {
            if (_RateLimit(reactorId, volId) == true)
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
    uint32_t nqnId = SpdkConnection::GetAttachedSubsystemId(volName.c_str());
    uint32_t i = 0;
    for (i = 0; i < nqnVolumeMap[nqnId].size(); i++)
    {
        if (volID == nqnVolumeMap[nqnId][i])
        {
            nqnVolumeMap[nqnId].erase(nqnVolumeMap[nqnId].begin() + i);
        }
    }
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
    double offset = (double)(now - next_tick) / param->qosTimeSlice;
    offset = offset + 1.0;
    param->nextTimeStamp = now + param->qosTimeSlice;
    if (SpdkConnection::IsReactorConnectionChanged(reactor) == true)
    {
        SpdkConnection::ResetReactorConnection(reactor);
        volList[reactor].clear();
        for (auto it = nqnVolumeMap.begin(); it != nqnVolumeMap.end(); it++)
        {
            uint32_t subsys = it->first;
            if (SpdkConnection::SpdkNvmfGetReactorSubsystemMapping(reactor, subsys) != M_INVALID_SUBSYSTEM)
            {
                volList[reactor][subsys] = GetVolumeFromActiveSubsystem(subsys);
            }
        }
    }
    for (auto subsystem = volList[reactor].begin(); subsystem != volList[reactor].end(); subsystem++)
    {
        std::vector<int> volumeList = volList[reactor][subsystem->first];
        for (uint32_t i = 0; i < volumeList.size(); i++)
        {
            int volId = volumeList[i];
            // Currently Qos supports just 1 Volume per subsystem, so default considers only 1st volume of subsystem list
            // Fix for memory leak issue SHPOS-410
            if (volId == volumeList[0])
            {
                _EnqueueVolumeParameter(reactor, volId, offset);
            }
            currentBW = 0;
            currentIO = 0;
            _ResetRateLimit(reactor, volId, offset);
            while (1)
            {
                if (_RateLimit(reactor, volId) == true)
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
            }
            volumeQosParam[reactor][volId].currentBW = currentBW;
            volumeQosParam[reactor][volId].currentIOs = currentIO;
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
    volumeQosParam[reactor][volId].valid = M_VALID_ENTRY;
    volumeQosParam[reactor][volId].currentBW = volumeQosParam[reactor][volId].currentBW / offset;
    volumeQosParam[reactor][volId].currentIOs = volumeQosParam[reactor][volId].currentIOs / offset;
    _EnqueueParams(reactor, volId, volumeQosParam[reactor][volId]);
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

} // namespace pos
