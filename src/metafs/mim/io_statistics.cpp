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

#include "io_statistics.h"

#include <string>
#include <utility>
#include <vector>

#include "metafs_log.h"
#include "metafs_mutex.h"
#include "src/metafs/common/meta_file_util.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/mim/mio.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
IoStatistics::IoStatistics(MetaFsConfigManager* configManager, MetaFsTimeInterval* interval, TelemetryPublisher* tp)
: telemetryPublisher(tp),
  sampledTimeSpentProcessingAllStages{},
  sampledTimeSpentFromIssueToComplete{},
  totalProcessedMioCount{},
  sampledProcessedMioCount{},
  metaFsTimeInterval(interval),
  skipCount(0),
  SAMPLING_SKIP_COUNT(configManager->GetSamplingSkipCount()),
  issueCountByStorage(),
  issueCountByFileType()
{
    if (!interval)
    {
        metaFsTimeInterval = new MetaFsTimeInterval(configManager->GetTimeIntervalInMillisecondsForMetric());
    }
}

IoStatistics::~IoStatistics(void)
{
    if (metaFsTimeInterval)
    {
        delete metaFsTimeInterval;
        metaFsTimeInterval = nullptr;
    }
}

void
IoStatistics::UpdateSubmissionMetrics(const Mio* mio)
{
    uint32_t ioType = mio->IsRead() ? (uint32_t)MetaIoRequestType::Read : (uint32_t)MetaIoRequestType::Write;
    issueCountByStorage[(int)mio->GetTargetStorage()][ioType]++;
    issueCountByFileType[(int)mio->GetFileType()][ioType]++;
}

void
IoStatistics::UpdateCompletionMetricsConditionally(const Mio* mio)
{
    uint32_t ioType = mio->IsRead() ? (uint32_t)MetaIoRequestType::Read : (uint32_t)MetaIoRequestType::Write;
    totalProcessedMioCount[ioType]++;

    if (skipCount++ % SAMPLING_SKIP_COUNT != 0)
    {
        sampledTimeSpentProcessingAllStages[ioType] += mio->GetElapsedInMilli(MioTimestampStage::Allocate, MioTimestampStage::Release).count();
        sampledTimeSpentFromIssueToComplete[ioType] += mio->GetElapsedInMilli(MioTimestampStage::Issue, MioTimestampStage::Complete).count();
        sampledProcessedMioCount[ioType]++;
        skipCount = 0;
    }
}

void
IoStatistics::PublishPeriodicMetrics(const size_t freeMioCount, const size_t cacheSize)
{
    if (telemetryPublisher && metaFsTimeInterval->CheckInterval())
    {
        POSMetricVector* metricVector = new POSMetricVector();

        POSMetric mFreeMioCount(TEL40200_METAFS_FREE_MIO_CNT, POSMetricTypes::MT_GAUGE);
        mFreeMioCount.SetGaugeValue(freeMioCount);
        metricVector->emplace_back(mFreeMioCount);

        POSMetric metricIoStatisticsWorking(TEL40205_METAFS_MIO_HANDLER_IS_WORKING, POSMetricTypes::MT_GAUGE);
        metricIoStatisticsWorking.SetGaugeValue(GetCurrDateTimestamp());
        metricVector->emplace_back(metricIoStatisticsWorking);

        for (uint32_t ioType = 0; ioType < NUM_IO_TYPE; ++ioType)
        {
            if (totalProcessedMioCount[ioType])
            {
                POSMetric m(TEL40302_METAFS_PROCESSED_MIO_COUNT, POSMetricTypes::MT_GAUGE);
                m.SetGaugeValue(totalProcessedMioCount[ioType]);
                m.AddLabel("direction", MetaFileUtil::ConvertToDirectionName(ioType));
                metricVector->emplace_back(m);
                totalProcessedMioCount[ioType] = 0;
            }
        }

        {
            POSMetric m(TEL40306_METAFS_CACHED_MPIO_COUNT, POSMetricTypes::MT_GAUGE);
            m.SetGaugeValue(cacheSize);
            metricVector->emplace_back(m);
        }

        for (uint32_t idx = 0; idx < NUM_STORAGE_TYPE; idx++)
        {
            for (uint32_t ioType = 0; ioType < NUM_IO_TYPE; ++ioType)
            {
                POSMetric m(TEL40103_METAFS_WORKER_ISSUE_COUNT_PARTITION, POSMetricTypes::MT_GAUGE);
                m.AddLabel("type", std::to_string(idx));
                m.AddLabel("direction", MetaFileUtil::ConvertToDirectionName(ioType));
                m.SetGaugeValue(issueCountByStorage[idx][ioType]);
                metricVector->emplace_back(m);
                issueCountByStorage[idx][ioType] = 0;
            }
        }

        for (uint32_t idx = 0; idx < NUM_FILE_TYPE; idx++)
        {
            for (uint32_t ioType = 0; ioType < NUM_IO_TYPE; ++ioType)
            {
                POSMetric m(TEL40105_METAFS_WORKER_ISSUE_COUNT_FILE_TYPE, POSMetricTypes::MT_GAUGE);
                m.AddLabel("type", std::to_string(idx));
                m.AddLabel("direction", MetaFileUtil::ConvertToDirectionName(ioType));
                m.SetGaugeValue(issueCountByFileType[idx][ioType]);
                metricVector->emplace_back(m);
                issueCountByFileType[idx][ioType] = 0;
            }
        }

        if (sampledProcessedMioCount)
        {
            for (uint32_t ioType = 0; ioType < NUM_IO_TYPE; ++ioType)
            {
                POSMetric mTimeSpentAllStage(TEL40301_METAFS_MIO_TIME_SPENT_PROCESSING_ALL_STAGES, POSMetricTypes::MT_GAUGE);
                mTimeSpentAllStage.SetGaugeValue(sampledTimeSpentProcessingAllStages[ioType]);
                mTimeSpentAllStage.AddLabel("direction", MetaFileUtil::ConvertToDirectionName(ioType));
                metricVector->emplace_back(mTimeSpentAllStage);

                POSMetric mTimeSpentIssueToComplete(TEL40203_METAFS_MIO_TIME_FROM_ISSUE_TO_COMPLETE, POSMetricTypes::MT_GAUGE);
                mTimeSpentIssueToComplete.SetGaugeValue(sampledTimeSpentFromIssueToComplete[ioType]);
                mTimeSpentIssueToComplete.AddLabel("direction", MetaFileUtil::ConvertToDirectionName(ioType));
                metricVector->emplace_back(mTimeSpentIssueToComplete);

                POSMetric m(TEL40204_METAFS_MIO_SAMPLED_COUNT, POSMetricTypes::MT_GAUGE);
                m.SetGaugeValue(sampledProcessedMioCount[ioType]);
                m.AddLabel("direction", MetaFileUtil::ConvertToDirectionName(ioType));
                metricVector->emplace_back(m);

                sampledTimeSpentProcessingAllStages[ioType] = 0;
                sampledTimeSpentFromIssueToComplete[ioType] = 0;
                sampledProcessedMioCount[ioType] = 0;
            }
        }

        telemetryPublisher->PublishMetricList(metricVector);
    }
}
std::vector<int64_t>
IoStatistics::GetSampledTimeSpentProcessingAllStages(void)
{
    auto& arr = sampledTimeSpentProcessingAllStages;
    return std::vector<int64_t>(std::begin(arr), std::end(arr));
}

std::vector<int64_t>
IoStatistics::GetSampledTimeSpentFromIssueToComplete(void)
{
    auto& arr = sampledTimeSpentFromIssueToComplete;
    return std::vector<int64_t>(std::begin(arr), std::end(arr));
}

std::vector<int64_t>
IoStatistics::GetTotalProcessedMioCount(void)
{
    auto& arr = totalProcessedMioCount;
    return std::vector<int64_t>(std::begin(arr), std::end(arr));
}

std::vector<int64_t>
IoStatistics::GetSampledProcessedMioCount(void)
{
    auto& arr = sampledProcessedMioCount;
    return std::vector<int64_t>(std::begin(arr), std::end(arr));
}

std::vector<std::vector<int64_t>>
IoStatistics::GetIssueCountByStorage(void)
{
    std::vector<std::vector<int64_t>> arr(NUM_STORAGE_TYPE, std::vector<int64_t>(NUM_IO_TYPE, 0));
    for (uint32_t i = 0; i < NUM_STORAGE_TYPE; ++i)
    {
        for (uint32_t j = 0; j < NUM_IO_TYPE; ++j)
        {
            arr[i][j] = issueCountByStorage[i][j];
        }
    }
    return arr;
}

std::vector<std::vector<int64_t>>
IoStatistics::GetIssueCountByFileType(void)
{
    std::vector<std::vector<int64_t>> arr(NUM_FILE_TYPE, std::vector<int64_t>(NUM_IO_TYPE, 0));
    for (uint32_t i = 0; i < NUM_FILE_TYPE; ++i)
    {
        for (uint32_t j = 0; j < NUM_IO_TYPE; ++j)
        {
            arr[i][j] = issueCountByFileType[i][j];
        }
    }
    return arr;
}
} // namespace pos
