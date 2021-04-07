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

#ifndef __IBOFOS_QOS_DATA_MANAGER_H__
#define __IBOFOS_QOS_DATA_MANAGER_H__

#include <queue>

#include "src/io/general_io/volume_io.h"
#include "src/qos/qos_common.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_scheduler.h"

namespace ibofos
{
class Ubio;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Interface Class for event scheduling policy
 *
 */
/* --------------------------------------------------------------------------*/
class QosDataManager
{
public:
    QosDataManager();
    ~QosDataManager();
    void EnqueueEventParams(uint32_t workerId, BackendEvent event, event_qos_params& event_param);
    event_qos_params DequeueEventParams(uint32_t workerId, BackendEvent event);
    void EnqueueVolumeParams(uint32_t reactor, uint32_t volId, volume_qos_params& volume_param);
    volume_qos_params DequeueVolumeParams(uint32_t reactor, uint32_t volId);
    void EnqueueEventUbio(uint32_t ioWorkerId, BackendEvent event, UbioSmartPtr ubio);
    void* PeekEventParams(uint32_t event);
    void EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, VolumeIoSmartPtr volIo);
    VolumeIoSmartPtr DequeueVolumeUbio(uint32_t reactorId, uint32_t volId);
    VolumeIoSmartPtr PeekVolumeUbio(uint32_t reactorId, uint32_t volId);
    UbioSmartPtr DequeueEventUbio(uint32_t ioWorkerId, uint32_t event);
    UbioSmartPtr PeekEventUbio(uint32_t ioWorkerId, uint32_t event);
    uint32_t GetEventWeight(uint32_t event);
    void SetEventWeight(uint32_t event, uint32_t weight);
    void SetEventWeightWRR(BackendEvent event, int64_t weight);
    int64_t GetEventWeightWRR(BackendEvent event);

private:
    // Events Realted Variables
    std::queue<event_qos_params> eventsParamsQueue[MAX_IO_WORKER][BackendEvent_Count];
    std::queue<UbioSmartPtr> eventsUbioQueue[MAX_IO_WORKER][BackendEvent_Count];
    std::mutex eventQueueLock[MAX_IO_WORKER][BackendEvent_Count];
    // Volume Related Variables
    std::queue<volume_qos_params> volumesParamsQueue[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::queue<VolumeIoSmartPtr> volumesUbioQueue[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::mutex volQueueLock[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    // Backend Event Priorities
    uint64_t eventWeight[BackendEvent_Count];
    std::atomic<int64_t> eventWeightWRR[BackendEvent_Count];
};

} // namespace ibofos

#endif // __IBOFOS_QOS_DATA_MANAGER_H__
