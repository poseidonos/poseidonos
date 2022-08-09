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

#include <functional>
#include <list>
#include <mutex>
#include <type_traits>

#include "Air.h"
#include "src/telemetry/telemetry_client/pos_metric.h"
#include "uptime_metric_generator.h"

namespace pos
{
class TelemetryPublisher;

template<typename T>
T
ToPrimitive(const air::JSONdoc& data)
{
    std::stringstream stream;
    T result;
    stream << data;
    stream >> result;
    return result;
}

enum CustomLabel : int
{
    None,
    ArrayIdVolumeId,
    SsdId,
    PortNumber,
};

struct AirMetricInfo
{
    std::string airNodeName;
    std::string airFilter{""};
    bool airPeriod{true};
    std::string airValue;
    std::string telemetryID;
    POSMetricTypes metricType;
    CustomLabel customLabel{CustomLabel::None};
};

class TelemetryAirDelegator
{
public:
    enum State : int
    {
        RUN = 0,
        END,
        ERR_DATA
    };
    explicit TelemetryAirDelegator(
        TelemetryPublisher* telPub,
        UptimeMetricGenerator* g = new UptimeMetricGenerator());
    virtual ~TelemetryAirDelegator(void);
    void SetState(State state);
    void RegisterAirEvent(void);
    std::function<int(const air::JSONdoc&& data)> dataHandler;

private:
    void PublishTimeTriggeredMetric(POSMetricVector* posMetricVector);

    TelemetryPublisher* telPub{nullptr};
    UptimeMetricGenerator* uptimeMetricGenerator{nullptr};
    int returnState{State::RUN};
    std::mutex mutex;

    std::list<AirMetricInfo> airMetricInfoList{
        {"PERF_ARR_VOL", "AIR_READ", true, "iops", TEL50000_READ_IOPS, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},
        {"PERF_ARR_VOL", "AIR_READ", true, "bw", TEL50001_READ_RATE_BYTES_PER_SECOND, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},
        {"PERF_ARR_VOL", "AIR_WRITE", true, "iops", TEL50010_WRITE_IOPS, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},
        {"PERF_ARR_VOL", "AIR_WRITE", true, "bw", TEL50011_WRITE_RATE_BYTES_PER_SECOND, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},

        {"PERF_PORT", "AIR_READ", true, "iops", TEL120001_READ_IOPS_PER_PORT, POSMetricTypes::MT_GAUGE, CustomLabel::PortNumber},
        {"PERF_PORT", "AIR_READ", true, "bw", TEL120002_READ_RATE_BYTES_PER_SECOND_PER_PORT, POSMetricTypes::MT_GAUGE, CustomLabel::PortNumber},
        {"PERF_PORT", "AIR_WRITE", true, "iops", TEL120011_WRITE_IOPS_PER_PORT, POSMetricTypes::MT_GAUGE, CustomLabel::PortNumber},
        {"PERF_PORT", "AIR_WRITE", true, "bw", TEL120012_WRITE_RATE_BYTES_PER_SECOND_PER_PORT, POSMetricTypes::MT_GAUGE, CustomLabel::PortNumber},

        {"PERF_SSD_Read", "AIR_UNKNOWN", true, "iops", TEL20000_READ_UNKNOWN_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_UNKNOWN", true, "bw", TEL20006_READ_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_JOURNAL", true, "iops", TEL20024_READ_JOURNAL_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_JOURNAL", true, "bw", TEL20025_READ_JOURNAL_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_META", true, "iops", TEL20001_READ_META_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_META", true, "bw", TEL20007_READ_META_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_GC", true, "iops", TEL20002_READ_GC_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_GC", true, "bw", TEL20008_READ_GC_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_HOST", true, "iops", TEL20003_READ_HOST_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_HOST", true, "bw", TEL20009_READ_HOST_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_FLUSH", true, "iops", TEL20004_READ_FLUSH_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_FLUSH", true, "bw", TEL20010_READ_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_REBUILD", true, "iops", TEL20005_READ_REBUILD_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Read", "AIR_REBUILD", true, "bw", TEL20011_READ_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},

        {"PERF_SSD_Write", "AIR_UNKNOWN", true, "iops", TEL20012_WRITE_UNKNOWN_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_UNKNOWN", true, "bw", TEL20018_WRITE_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_JOURNAL", true, "iops", TEL20026_WRITE_JOURNAL_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_JOURNAL", true, "bw", TEL20027_WRITE_JOURNAL_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_META", true, "iops", TEL20013_WRITE_META_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_META", true, "bw", TEL20019_WRITE_META_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_GC", true, "iops", TEL20014_WRITE_GC_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_GC", true, "bw", TEL20020_WRITE_GC_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_HOST", true, "iops", TEL20015_WRITE_HOST_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_HOST", true, "bw", TEL20021_WRITE_HOST_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_FLUSH", true, "iops", TEL20016_WRITE_FLUSH_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_FLUSH", true, "bw", TEL20022_WRITE_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_REBUILD", true, "iops", TEL20017_WRITE_REBUILD_IOPS_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},
        {"PERF_SSD_Write", "AIR_REBUILD", true, "bw", TEL20023_WRITE_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD, POSMetricTypes::MT_GAUGE, CustomLabel::SsdId},

        {"LAT_ARR_VOL_READ", "", true, "mean", TEL50002_READ_LATENCY_MEAN_NS, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},
        {"LAT_ARR_VOL_READ", "", true, "max", TEL50003_READ_LATENCY_MAX_NS, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},
        {"LAT_ARR_VOL_WRITE", "", true, "mean", TEL50012_WRITE_LATENCY_MEAN_NS, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},
        {"LAT_ARR_VOL_WRITE", "", true, "max", TEL50013_WRITE_LATENCY_MAX_NS, POSMetricTypes::MT_GAUGE, CustomLabel::ArrayIdVolumeId},

        {"UTIL_REACTOR", "", true, "usage", TEL70000_SPDK_REACTOR_UTILIZATION, POSMetricTypes::MT_GAUGE},

        {"CNT_PendingIO", "", true, "count", TEL80000_DEVICE_PENDING_IO_COUNT, POSMetricTypes::MT_GAUGE},

        {"VolumeIo_Constructor", "", false, "count", TEL130001_COUNT_OF_VOLUME_IO_CONSTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"VolumeIo_Destructor", "", false, "count", TEL130002_COUNT_OF_VOLUME_IO_DESTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"Ubio_Constructor", "", false, "count", TEL130003_COUNT_OF_UBIO_CONSTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"Ubio_Destructor", "", false, "count", TEL130004_COUNT_OF_UBIO_DESTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"SSD_Submit", "", false, "count", TEL130005_SUBMISSION_COUNT_OF_SSD_IOS, POSMetricTypes::MT_GAUGE},
        {"SSD_Complete", "", false, "count", TEL130006_COMPLETION_COUNT_OF_SSD_IOS, POSMetricTypes::MT_GAUGE},
        {"EventQueue_Push", "", false, "count", TEL130007_PUSHING_COUNT_OF_EVENT_QUEUE, POSMetricTypes::MT_GAUGE},
        {"WorkerCommonQueue_Push", "", false, "count", TEL130008_PUSHING_COUNT_OF_WORKER_COMMON_QUEUE, POSMetricTypes::MT_GAUGE},
        {"WorkerCommonQueue_Pop", "", false, "count", TEL130009_POPPING_COUNT_OF_WORKER_COMMON_QUEUE, POSMetricTypes::MT_GAUGE},
        {"Callback_Constructor", "", false, "count", TEL130010_COUNT_OF_CALLBACK_CONSTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"Callback_Destructor", "", false, "count", TEL130011_COUNT_OF_CALLBACK_DESTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"Event_Constructor", "", false, "count", TEL130012_COUNT_OF_EVENT_CONSTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"Event_Destructor", "", false, "count", TEL130013_COUNT_OF_EVENT_DESTRUCTORS, POSMetricTypes::MT_GAUGE},
        {"IOWorker_Submit", "", false, "count", TEL130014_SUBMISSION_COUNT_IN_IO_WORKER, POSMetricTypes::MT_GAUGE},
        {"IOWorker_Complete", "", false, "count", TEL130015_COMPLETION_COUNT_IN_IO_WORKER, POSMetricTypes::MT_GAUGE},
        {"RequestedUserRead", "", false, "count", TEL140001_COUNT_OF_REQUSTED_USER_READ, POSMetricTypes::MT_GAUGE},
        {"RequestedUserWrite", "", false, "count", TEL140002_COUNT_OF_REQUSTED_USER_WRITE, POSMetricTypes::MT_GAUGE},
        {"RequestedUserAdminIo", "", false, "count", TEL140003_COUNT_OF_REQUSTED_USER_ADMINIO, POSMetricTypes::MT_GAUGE},
        {"CompleteUserRead", "", false, "count", TEL140004_COUNT_OF_COMPLETE_USER_READ, POSMetricTypes::MT_GAUGE},
        {"CompleteUserWrite", "", false, "count", TEL140005_COUNT_OF_COMPLETE_USER_WRITE, POSMetricTypes::MT_GAUGE},
        {"CompleteUserAdminIo", "", false, "count", TEL140006_COUNT_OF_COMPLETE_USER_ADMINIO, POSMetricTypes::MT_GAUGE},
        {"UserFlushProcess", "", false, "count", TEL140007_COUNT_OF_USER_FLUSH_PROCESS, POSMetricTypes::MT_GAUGE},
        {"PartialWriteProcess", "", false, "count", TEL140008_COUNT_OF_PARTIAL_WRITE_PROCESS, POSMetricTypes::MT_GAUGE},
        {"UserFailIo", "", false, "count", TEL140009_COUNT_OF_USER_FAIL_IO, POSMetricTypes::MT_GAUGE},
        {"UserReadPendingCnt", "", false, "count", TEL140010_COUNT_OF_USER_READ_PENDING_CNT, POSMetricTypes::MT_GAUGE},
        {"UserWritePendingCnt", "", false, "count", TEL140011_COUNT_OF_USER_WRITE_PENDING_CNT, POSMetricTypes::MT_GAUGE},
        {"InternalIoPendingCnt", "", false, "count", TEL140012_COUNT_OF_INTERNAL_IO_PENDING_CNT, POSMetricTypes::MT_GAUGE},
    };
};
} // namespace pos
