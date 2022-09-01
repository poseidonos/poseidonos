/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <sched.h>

#include <unordered_map>
#include <string>
#include <vector>

#include "src/metafs/mim/metafs_io_wrr_q.h"
#include "src/metafs/mim/scalable_meta_io_worker.h"

namespace pos
{
class TelemetryPublisher;
class MetaFsConfigManager;
class MetaFsTimeInterval;

class MetaFsIoScheduler : public MetaFsIoHandlerBase
{
public:
    // only for test
    MetaFsIoScheduler(void);
    explicit MetaFsIoScheduler(const int threadId, const int coreId,
        const int totalCoreCount, const std::string& threadName,
        const cpu_set_t mioCoreSet, MetaFsConfigManager* config,
        TelemetryPublisher* tp, MetaFsTimeInterval* timeInterval,
        const std::vector<int> weight, const bool supportNumaDedicatedScheduling);
    virtual ~MetaFsIoScheduler(void);

    void IssueRequestAndDelete(MetaFsIoRequest* reqMsg);
    virtual void EnqueueNewReq(MetaFsIoRequest* reqMsg);

    virtual bool AddArrayInfo(const int arrayId, const MaxMetaLpnMapPerMetaStorage& map) override;
    virtual bool RemoveArrayInfo(const int arrayId) override;

    virtual void StartThread(void) override;
    virtual void ExitThread(void) override;
    void Execute(void);

    void RegisterMetaIoWorkerForTest(ScalableMetaIoWorker* metaIoWorker);

    // for test
    const int64_t* GetIssueCount(size_t& size /* output */) const;

private:
    MetaFsIoRequest* _FetchPendingNewReq(void);
    void _CreateMioThread(void);
    void _SetRequestCountOrCallbackCountOfCurrentRequest(int count);
    void _SetCurrentContextFrom(MetaFsIoRequest* reqMsg);
    void _ClearCurrentContext(void);
    void _UpdateCurrentLpnToNextExtent(void);
    void _UpdateCurrentLpnToNextExtentConditionally(void);
    void _PushToMioThreadList(const uint32_t coreId, ScalableMetaIoWorker* worker);
    void _IssueRequestToMioWorker(MetaFsIoRequest* reqMsg);
    bool _DoesMioWorkerForNumaExist(const int numaId);
    uint32_t _GetNumaIdConsideringNumaDedicatedScheduling(const uint32_t numaId);
    uint32_t _GetIndexOfWorkerConsideringNumaDedicatedScheduling(const uint32_t numaId);

    const size_t TOTAL_NUMA_COUNT;
    const bool SUPPORT_NUMA_DEDICATED_SCHEDULING;
    const size_t TOTAL_CORE_COUNT;
    const cpu_set_t MIO_CORE_SET;
    std::unordered_map<uint32_t, std::vector<ScalableMetaIoWorker*>> metaIoWorkerList_;
    size_t mioCoreCount_;
    std::vector<size_t> mioCoreCountInTheSameNuma_;
    MetaFsConfigManager* config_;
    TelemetryPublisher* tp_;
    size_t cpuStallCnt_;
    MetaFsTimeInterval* timeInterval_;
    const size_t MAX_CPU_STALL_COUNT = 1000;

    MetaFsIoWrrQ<MetaFsIoRequest*, MetaFileType> ioSQ_;
    MetaFsIoRequest* currentReqMsg_;
    FileSizeType chunkSize_;
    MetaLpnType fileBaseLpn_;
    MetaLpnType startLpn_;
    MetaLpnType currentLpn_;
    size_t requestCount_;
    size_t remainCount_;
    int extentsCount_;
    int currentExtent_;
    MetaFileExtent* extents_;
    std::vector<int> weight_;
    bool needToIgnoreNuma_;

    static const uint32_t NUM_STORAGE = (int)MetaStorageType::Max;
    int64_t issueCount_[NUM_STORAGE];
    std::string metricNameForStorage_[NUM_STORAGE];
};
} // namespace pos
