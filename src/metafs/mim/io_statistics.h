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

#include <string>
#include <vector>

#include "src/metafs/include/meta_file_property.h"
#include "src/metafs/include/meta_storage_specific.h"
#include "src/metafs/lib/metafs_time_interval.h"
#include "src/metafs/mim/metafs_io_request.h"
#include "src/metafs/util/metafs_time.h"

namespace pos
{
class Mio;
class MetaFsConfigManager;
class TelemetryPublisher;

class IoStatistics
{
public:
    IoStatistics(MetaFsConfigManager* configManager, MetaFsTimeInterval* interval, TelemetryPublisher* tp);
    virtual ~IoStatistics(void);

    virtual void UpdateSubmissionMetrics(const Mio* mio);
    virtual void UpdateCompletionMetricsConditionally(const Mio* mio);
    virtual void PublishPeriodicMetrics(const size_t freeMioCount, const size_t cacheSize);

    // for test
    std::vector<int64_t> GetSampledTimeSpentProcessingAllStages(void);
    std::vector<int64_t> GetSampledTimeSpentFromIssueToComplete(void);
    std::vector<int64_t> GetTotalProcessedMioCount(void);
    std::vector<int64_t> GetSampledProcessedMioCount(void);
    std::vector<std::vector<int64_t>> GetIssueCountByStorage(void);
    std::vector<std::vector<int64_t>> GetIssueCountByFileType(void);

private:
    static const uint32_t NUM_STORAGE_TYPE = (int)MetaStorageType::Max;
    static const uint32_t NUM_FILE_TYPE = (int)MetaFileType::MAX;
    static const uint32_t NUM_IO_TYPE = (int)MetaIoRequestType::Max;

    TelemetryPublisher* telemetryPublisher;
    int64_t sampledTimeSpentProcessingAllStages[NUM_IO_TYPE];
    int64_t sampledTimeSpentFromIssueToComplete[NUM_IO_TYPE];
    int64_t totalProcessedMioCount[NUM_IO_TYPE];
    int64_t sampledProcessedMioCount[NUM_IO_TYPE];
    MetaFsTimeInterval* metaFsTimeInterval;
    size_t skipCount;
    const size_t SAMPLING_SKIP_COUNT;

    int64_t issueCountByStorage[NUM_STORAGE_TYPE][NUM_IO_TYPE];
    int64_t issueCountByFileType[NUM_FILE_TYPE][NUM_IO_TYPE];
};
} // namespace pos
