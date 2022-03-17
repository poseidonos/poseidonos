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

#pragma once

#include <atomic>
#include <map>
#include <pthread.h>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <list>
#include "src/bio/volume_io.h"

#include "src/spdk_wrapper/caller/spdk_pos_nvmf_caller.h"
#include "src/spdk_wrapper/caller/spdk_pos_volume_caller.h"
#include "src/qos/exit_handler.h"
#include "src/qos/qos_array_manager.h"
#include "src/qos/qos_common.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/sys_event/volume_event.h"
#include "src/sys_event/volume_event_publisher.h"
namespace pos
{
class QosManager;
class IbofIoSubmissionAdapter;
class BwIopsRateLimit;
class ParameterQueue;
template<class T>
class IoQueue;
class QosVolumeManager : public VolumeEvent, public ExitQosHandler
{
public:
    QosVolumeManager(QosContext* qosCtx, bool feQos, uint32_t arrayIndex,
        QosArrayManager* qosArrayManager,
        EventFrameworkApi* eventFrameworkApiArg,
        QosManager* qosManager,
        SpdkPosNvmfCaller* spdkPosNvmfCaller = new SpdkPosNvmfCaller(),
        SpdkPosVolumeCaller* spdkPosVolumeCaller = new SpdkPosVolumeCaller(),
        VolumeEventPublisher* volumeEventPublisher = VolumeEventPublisherSingleton::Instance());
    ~QosVolumeManager(void) override;
    int VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    int VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    int VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) override;
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId, bool withLock = true);
    void HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, VolumeIoSmartPtr io);
    int VolumeQosPoller(IbofIoSubmissionAdapter* aioSubmission, double offset);
    void SetVolumeLimit(uint32_t volId, int64_t weight, bool iops);
    int64_t GetVolumeLimit(uint32_t volId, bool iops);
    void DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId);
    void GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subsysVolMap);
    void ResetRateLimit(uint32_t reactor, int volId, double offset);
    std::string GetArrayName(void);
    void SetArrayName(std::string arrayName);
    void ResetVolumeThrottling(int volId, uint32_t arrayId);
    static void _VolumeMountHandler(void* arg1, void* arg2);
    static void _VolumeUnmountHandler(void* arg1, void* arg2);
    static void _VolumeDetachHandler(void* arg1, void* arg2);
    void GetMountedVolumes(std::list<uint32_t>& volumeList);
    void SetMinimumVolume(uint32_t volId, uint64_t value, bool iops);
    uint64_t GetDynamicVolumeThrottling(uint32_t volId, bool iops);

    static uint64_t GetGlobalThrottling(bool iops);
    static void ResetGlobalThrottling(void);
    static void SetRemainingThrottling(uint64_t total, uint64_t minVolTotal, bool iops);

    static std::atomic<int64_t> globalBwThrottling;
    static std::atomic<int64_t> globalIopsThrottling;
    static std::atomic<int64_t> globalRemainingVolumeBw;
    static std::atomic<int64_t> globalRemainingVolumeIops;

    void EnqueueVolumeIo(uint32_t volId, VolumeIoSmartPtr io);
    VolumeIoSmartPtr DequeueVolumeIo(uint32_t volId);
    void SubmitVolumeIoToAio(IbofIoSubmissionAdapter* aioSubmission, uint32_t volId, VolumeIoSmartPtr volumeIo);

protected:
    EventFrameworkApi* eventFrameworkApi;

private:
    bool _RateLimit(int volId);
    bool _GlobalRateLimit(void);
    bool _SpecialRateLimit(uint32_t volId);
    bool _MinimumRateLimit(int volId);

    void _UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw, std::string arrayName);
    void _EnqueueVolumeParameter(uint32_t reactor, uint32_t volId, double offset);
    void _ClearVolumeParameters(uint32_t volId);

    void _InternalVolMountHandlerQos(struct pos_volume_info* volMountInfo);
    void _InternalVolUnmountHandlerQos(struct pos_volume_info* volUnmountInfo);
    void _InternalVolDetachHandlerQos(struct pos_volume_info* volDetachInfo);
    void _CopyVolumeInfo(char* destInfo, const char* srcInfo, int len);
    bool _PollingAndSubmit(IbofIoSubmissionAdapter* aioSubmission, uint32_t volId);
    static int64_t _GetThrottlingChange(int64_t remainingValue, int64_t plusUpdateUnit, uint64_t minusUpdateUnit);
    static int64_t _ResetThrottlingCommon(int64_t remainingValue, uint64_t currentThrottlingValue);
    void _PrintWarningLogIfNotGuaranteed(uint32_t volId);
    void _CalculateMovingAverage(int volId);

    std::string _GetBdevName(uint32_t id, string arrayName);
    std::unordered_map<int32_t, std::vector<int>> nqnVolumeMap;
    std::map<uint32_t, vector<int>> volList[M_MAX_REACTORS];
    bw_iops_parameter volumeQosParam[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::atomic<int64_t> remainingDynamicVolumeBw[MAX_VOLUME_COUNT];
    std::atomic<int64_t> remainingDynamicVolumeIops[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> bwThrottling[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> iopsThrottling[MAX_VOLUME_COUNT];
    std::atomic<int64_t> dynamicBwThrottling[MAX_VOLUME_COUNT];
    std::atomic<int64_t> dynamicIopsThrottling[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> pendingIO[MAX_VOLUME_COUNT];

    std::atomic<bool> volumeMap[MAX_VOLUME_COUNT];
    std::mutex volumePendingIOLock[MAX_VOLUME_COUNT];

    int64_t remainingVolumeBw[MAX_VOLUME_COUNT];
    int64_t remainingVolumeIops[MAX_VOLUME_COUNT];
    std::string volumeName[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> minVolumeBw[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> minVolumeIops[MAX_VOLUME_COUNT];
    static std::atomic<int64_t> notThrottledVolumesThrottlingBw;
    static std::atomic<int64_t> remainingNotThrottledVolumesBw;
    static std::atomic<int64_t> notThrottledVolumesThrottlingIops;
    static std::atomic<int64_t> remainingNotThrottledVolumesIops;

    int64_t previousRemainingVolumeBw[MAX_VOLUME_COUNT];
    int64_t previousRemainingVolumeIops[MAX_VOLUME_COUNT];
    uint64_t avgBw[MAX_VOLUME_COUNT];
    uint64_t avgIops[MAX_VOLUME_COUNT];
    static const uint64_t AVG_PERF_PERIOD = PARAMETER_COLLECTION_INTERVAL;
    uint64_t minimumCheckCounter[MAX_VOLUME_COUNT];
    bool isLogPrinted[MAX_VOLUME_COUNT];
    uint64_t logPrintedCounter[MAX_VOLUME_COUNT];
    static const uint64_t LOG_PRINT_PERIOD = 60; // 60 sec

    bool feQosEnabled;
    BwIopsRateLimit* bwIopsRateLimit;
    ParameterQueue* parameterQueue;
    std::queue<VolumeIoSmartPtr> ioQueue[MAX_VOLUME_COUNT];
    QosContext* qosContext;
    QosArrayManager* qosArrayManager;
    QosManager* qosManager;
    std::mutex subsysVolMapLock;
    const char* BDEV_NAME_PREFIX = "bdev_";
    SpdkPosNvmfCaller* spdkPosNvmfCaller;
    SpdkPosVolumeCaller* spdkPosVolumeCaller;
    VolumeEventPublisher* volumeEventPublisher;
    pthread_rwlock_t nqnLock;

    static int64_t basicBwUnit;
    static int64_t basicIopsUnit;
    static float globalThrottlingChangingRate;
    float volumeThrottlingChangingRate;
    float minGuaranteedThrottlingRate;
    static int64_t globalThrottlingIncreaseCoefficient;
    int64_t minGuaranteedIncreaseCoefficient;
    float minThrottlingBiasedRate;
};
} // namespace pos
