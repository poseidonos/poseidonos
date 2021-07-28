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

#pragma once

#include <atomic>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include "src/qos/exit_handler.h"
#include "src/qos/qos_array_manager.h"
#include "src/qos/qos_common.h"
#include "src/sys_event/volume_event.h"

namespace pos
{
class IbofIoSubmissionAdapter;
class BwIopsRateLimit;
class ParameterQueue;
template<class T>
class IoQueue;
class QosVolumeManager : public VolumeEvent, public ExitQosHandler
{
public:
    QosVolumeManager(QosContext* qosCtx, bool feQos, uint32_t arrayIndex, QosArrayManager* qosArrayManager);
    ~QosVolumeManager(void);
    bool VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    bool VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    bool VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    bool VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    bool VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    bool VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    void VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) override;
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId);
    void HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, pos_io* io);
    bw_iops_parameter DequeueParams(uint32_t reactor, uint32_t volId);
    int VolumeQosPoller(uint32_t reactor, IbofIoSubmissionAdapter* aioSubmission, double offset);
    void SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t weight, bool iops);
    int64_t GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops);
    void DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId);
    void GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subsysVolMap);
    void ResetRateLimit(uint32_t reactor, int volId, double offset);
    std::string GetArrayName(void);
    void SetArrayName(std::string arrayName);

private:
    void _EnqueueParams(uint32_t reactor, uint32_t volId, bw_iops_parameter& volume_param);
    bool _RateLimit(uint32_t reactor, int volId);
    void _UpdateRateLimit(uint32_t reactor, int volId, uint64_t size);
    void _EnqueueVolumeUbio(uint32_t rectorId, uint32_t volId, pos_io* io);
    void _UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw, std::string arrayName);
    pos_io* _DequeueVolumeUbio(uint32_t reactorId, uint32_t volId);
    void _EnqueueVolumeParameter(uint32_t reactor, uint32_t volId, double offset);
    void _ClearVolumeParameters(uint32_t volId);
    std::string _GetBdevName(uint32_t id, string arrayName);
    std::unordered_map<int32_t, std::vector<int>> nqnVolumeMap;
    std::map<uint32_t, vector<int>> volList[M_MAX_REACTORS];
    bw_iops_parameter volumeQosParam[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::atomic<uint64_t> volReactorWeight[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::atomic<int64_t> volReactorIopsWeight[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    uint64_t pendingIO[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    bool feQosEnabled;
    BwIopsRateLimit* bwIopsRateLimit;
    ParameterQueue* parameterQueue;
    IoQueue<pos_io*>* ioQueue;
    QosContext* qosContext;
    QosArrayManager* qosArrayManager;
    std::mutex subsysVolMapLock;
    const char* BDEV_NAME_PREFIX = "bdev_";
};
} // namespace pos
