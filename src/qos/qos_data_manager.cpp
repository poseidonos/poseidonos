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

#include "src/qos/qos_data_manager.h"

#include <iostream>
using namespace std;
namespace ibofos
{
const uint16_t PRIORITY_HIGH_DEFAULT_WEIGHT = 10;
const uint16_t PRIORITY_LOW_DEFAULT_WEIGHT = 1;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosDataManager::QosDataManager()
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosDataManager::~QosDataManager()
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
QosDataManager::EnqueueEventParams(uint32_t workerId, BackendEvent event, event_qos_params& event_param)
{
    std::unique_lock<std::mutex> uniqueLock(eventQueueLock[workerId][event]);
    eventsParamsQueue[workerId][event].push(event_param);
}

event_qos_params
QosDataManager::DequeueEventParams(uint32_t workerId, BackendEvent event)
{
    event_qos_params ret = {
        0,
    };
    ret.valid = M_INVALID_ENTRY;
    std::unique_lock<std::mutex> uniqueLock(eventQueueLock[workerId][event]);
    if (eventsParamsQueue[workerId][event].empty() == false)
    {
        ret = eventsParamsQueue[workerId][event].front();
        eventsParamsQueue[workerId][event].pop();
    }
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosDataManager::EnqueueVolumeParams(uint32_t reactor, uint32_t volId,
    volume_qos_params& volume_param)
{
    std::unique_lock<std::mutex> uniqueLock(volQueueLock[reactor][volId]);
    volumesParamsQueue[reactor][volId].push(volume_param);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

volume_qos_params
QosDataManager::DequeueVolumeParams(uint32_t reactor, uint32_t volId)
{
    volume_qos_params ret = {
        0,
    };
    ret.valid = M_INVALID_ENTRY;
    std::unique_lock<std::mutex> uniqueLock(volQueueLock[reactor][volId]);
    if (volumesParamsQueue[reactor][volId].empty() == false)
    {
        ret = volumesParamsQueue[reactor][volId].front();
        volumesParamsQueue[reactor][volId].pop();
    }
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeIoSmartPtr
QosDataManager::DequeueVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    VolumeIoSmartPtr ret = nullptr;
    if (volumesUbioQueue[reactorId][volId].empty() == false)
    {
        ret = volumesUbioQueue[reactorId][volId].front();
        volumesUbioQueue[reactorId][volId].pop();
    }
    return ret;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeIoSmartPtr
QosDataManager::PeekVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    VolumeIoSmartPtr ret = nullptr;
    if (volumesUbioQueue[reactorId][volId].empty() == false)
    {
        ret = volumesUbioQueue[reactorId][volId].front();
    }
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
UbioSmartPtr
QosDataManager::DequeueEventUbio(uint32_t id, uint32_t event)
{
    UbioSmartPtr ret = nullptr;
    if (eventsUbioQueue[id][event].empty() == false)
    {
        ret = eventsUbioQueue[id][event].front();
        eventsUbioQueue[id][event].pop();
    }
    return ret;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
UbioSmartPtr
QosDataManager::PeekEventUbio(uint32_t id, uint32_t event)
{
    UbioSmartPtr ret = nullptr;
    if (eventsUbioQueue[id][event].empty() == false)
    {
        ret = eventsUbioQueue[id][event].front();
    }
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosDataManager::EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio)
{
    eventsUbioQueue[id][event].push(ubio);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosDataManager::EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, VolumeIoSmartPtr ubio)
{
    volumesUbioQueue[reactorId][volId].push(ubio);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosDataManager::GetEventWeight(uint32_t event)
{
    return eventWeight[event];
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosDataManager::SetEventWeight(uint32_t event, uint32_t weight)
{
    eventWeight[event] = weight;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

int64_t
QosDataManager::GetEventWeightWRR(BackendEvent event)
{
    return (eventWeightWRR[event]);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosDataManager::SetEventWeightWRR(BackendEvent event, int64_t weight)
{
    eventWeightWRR[event] = weight;
}
} // namespace ibofos
