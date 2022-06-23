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

#include "src/telemetry/telemetry_air/telemetry_air_delegator.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_info/space_info.h"
#include "src/telemetry/telemetry_client/pos_metric.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/telemetry/telemetry_id.h"
#include "src/volume/volume_service.h"
#include "src/telemetry/telemetry_client/pos_metric.h"

namespace pos
{

enum class PerformanceMetric
{
    GET_SSD_ID,
    ARR_VOLUME,
    PORT
};

void
AddLabelArrayVolumeInterval(POSMetric& posMetric,
    const air::JSONdoc& data, const std::string& interval)
{
    std::stringstream stream_index;
    stream_index << data["index"]; // 0x0205 (array id = 2, volume id = 5)
    uint64_t index{0};
    stream_index >> index;
    uint32_t array_id = (index & 0xFF00) >> 8;
    posMetric.AddLabel("array_id", std::to_string(array_id));

    uint32_t volume_id = (index & 0x00FF);
    posMetric.AddLabel("volume_id", std::to_string(volume_id));

    posMetric.AddLabel("interval", interval);
}

void
AddSSDIdInterval(POSMetric& posMetric,
    const air::JSONdoc& data, const std::string& interval)
{
    std::stringstream stream_index;
    stream_index << data["index"];
    uint64_t index{0};
    stream_index >> index;
    posMetric.AddLabel("SSD_id", std::to_string(index));

    posMetric.AddLabel("interval", interval);
}

std::string
DecodePort(uint64_t index)
{
    std::string portId[4], port;
    portId[0] = std::to_string(index & 0xFF);
    portId[1] = std::to_string((index & 0xFF00) >> 8);
    portId[2] = std::to_string((index & 0xFF0000) >> 16);
    portId[3] = std::to_string((index & 0xFF000000) >> 24);
    port = std::to_string(index >> 32);
    return (portId[0] + "." + portId[1] + "." + portId[2] + "." + portId[3] + ":" + port);
}

void
AddPortInterval(POSMetric& posMetric,
    const air::JSONdoc& data, const std::string& interval)
{
    std::stringstream stream_index;
    stream_index << data["index"];
    uint64_t index{0};
    stream_index >> index;
    posMetric.AddLabel("port", DecodePort(index));
}

void
AddPerformanceMetric(POSMetricVector* posMetricVector,
    const std::string& name, const POSMetricTypes type, const uint64_t value,
    const air::JSONdoc& data, const std::string& interval,
    PerformanceMetric perfMetric = PerformanceMetric::ARR_VOLUME)
{
    POSMetric posMetric{name, type};
    if (POSMetricTypes::MT_GAUGE == type)
    {
        posMetric.SetGaugeValue(value);
    }
    if (perfMetric == PerformanceMetric::GET_SSD_ID)
    {
        AddSSDIdInterval(posMetric, data, interval);
    }
    else if (perfMetric == PerformanceMetric::ARR_VOLUME)
    {
        AddLabelArrayVolumeInterval(posMetric, data, interval);
    }
    else
    {
        AddPortInterval(posMetric, data, interval);
    }
    
    std::stringstream stream_thread_id;
    stream_thread_id << data["target_id"];
    posMetric.AddLabel("thread_id", stream_thread_id.str());

    std::stringstream stream_thread_name;
    stream_thread_name << data["target_name"];
    posMetric.AddLabel("thread_name", stream_thread_name.str());

    posMetricVector->push_back(posMetric);
}

void
AddLatencyMetric(POSMetricVector* posMetricVector,
    const std::string& name, const POSMetricTypes type, const uint32_t value,
    const air::JSONdoc& data, const std::string& interval)
{
    POSMetric posMetric{name, type};
    if (POSMetricTypes::MT_GAUGE == type)
    {
        posMetric.SetGaugeValue(value);
    }

    AddLabelArrayVolumeInterval(posMetric, data, interval);

    std::stringstream stream_sample_count;
    stream_sample_count << data["period"]["sample_cnt"];
    posMetric.AddLabel("sample_count", stream_sample_count.str());

    posMetricVector->push_back(posMetric);
}

template<typename T>
void
AddCommonMetric(POSMetricVector* posMetricVector,
    const std::string& name, const POSMetricTypes type, const T value,
    const air::JSONdoc& data, const std::string& interval)
{
    POSMetric posMetric{name, type};
    if (POSMetricTypes::MT_GAUGE == type)
    {
        posMetric.SetGaugeValue(value);
    }

    std::stringstream stream_thread_id;
    stream_thread_id << data["target_id"];
    posMetric.AddLabel("thread_id", stream_thread_id.str());

    std::stringstream stream_thread_name;
    stream_thread_name << data["target_name"];
    posMetric.AddLabel("thread_name", stream_thread_name.str());

    std::stringstream stream_filter;
    stream_filter << data["filter"];
    posMetric.AddLabel("filter", stream_filter.str());

    std::stringstream stream_index;
    stream_index << data["index"];
    posMetric.AddLabel("index", stream_index.str());

    posMetricVector->push_back(posMetric);
}

void
AddUsageMetric(POSMetricVector* posMetricVector,
    const std::string& name, const POSMetricTypes type, const uint64_t value,
    uint32_t array_id, uint32_t volume_id)
{
    POSMetric posMetric{name, type};
    if (POSMetricTypes::MT_GAUGE == type)
    {
        posMetric.SetGaugeValue(value);
    }
    else if (POSMetricTypes::MT_COUNT == type)
    {
        posMetric.SetCountValue(value);
    }
    else
    {
        return;
    }
    posMetric.AddLabel("array_id", std::to_string(array_id));
    posMetric.AddLabel("volume_id", std::to_string(volume_id));

    posMetricVector->push_back(posMetric);
}

void
TelemetryAirDelegator::PublishTimeTriggeredMetric(POSMetricVector* posMetricVector)
{
    for (uint32_t arrayId = 0; arrayId < ArrayMgmtPolicy::MAX_ARRAY_CNT; arrayId++)
    {
        IVolumeManager* vm = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);
        if (vm != nullptr)
        {
            for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
            {
                VolumeBase* volume = vm->GetVolume(volId);
                if ((volume != nullptr) && (volume->GetStatus() == Mounted))
                {
                    uint64_t volUsage = volume->UsedSize();
                    AddUsageMetric(posMetricVector, TEL60003_VOL_USAGE_BLK_CNT,
                    POSMetricTypes::MT_GAUGE, volUsage, arrayId, volId);
                }
            }
        }
    }

    if (uptimeMetricGenerator != nullptr) {
        POSMetric uptimeMetric;
        int ret = uptimeMetricGenerator->Generate(&uptimeMetric);
        if (ret != -1 && posMetricVector != nullptr)
        {
            posMetricVector->push_back(uptimeMetric);
        }
    }
}

TelemetryAirDelegator::TelemetryAirDelegator(
    TelemetryPublisher* telPub,
    UptimeMetricGenerator* g)
: telPub(telPub),
  uptimeMetricGenerator(g)
{
    dataHandler = [this](const air::JSONdoc&& data) -> int {
        const std::lock_guard<std::mutex> lock(this->mutex);
        POSMetricVector* posMetricVector{nullptr};
        try
        {
            if (State::RUN == this->returnState)
            {
                posMetricVector = new POSMetricVector;

                std::string interval{"0"};
                if (data.HasKey("interval"))
                {
                    std::stringstream stream_interval;
                    stream_interval << data["interval"];
                    uint32_t num_interval{0};
                    stream_interval >> num_interval;
                    interval = std::to_string(num_interval);
                }

                if (data.HasKey("PERF_ARR_VOL"))
                {
                    auto& objs = data["PERF_ARR_VOL"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_filter;
                        stream_filter << obj["filter"];
                        std::string str_filter = stream_filter.str();

                        std::stringstream stream_iops;
                        stream_iops << obj["period"]["iops"];
                        uint32_t iops{0};
                        stream_iops >> iops;

                        std::stringstream stream_bw;
                        stream_bw << obj["period"]["bw"];
                        uint64_t bw{0};
                        stream_bw >> bw;

                        std::string iopsMetricId;
                        std::string bwMetricId;

                        if (0 == str_filter.compare("\"AIR_READ\""))
                        {
                            iopsMetricId = TEL50000_READ_IOPS;
                            bwMetricId = TEL50001_READ_RATE_BYTES_PER_SECOND;
                        }
                        else
                        {
                            iopsMetricId = TEL50010_WRITE_IOPS;
                            bwMetricId = TEL50011_WRITE_RATE_BYTES_PER_SECOND;
                        }
                        AddPerformanceMetric(posMetricVector, iopsMetricId,
                            POSMetricTypes::MT_GAUGE, iops, obj, interval);
                        AddPerformanceMetric(posMetricVector, bwMetricId,
                            POSMetricTypes::MT_GAUGE, bw, obj, interval);
                    }
                }

                if (data.HasKey("PERF_PORT"))
                {
                    auto& objs = data["PERF_PORT"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_filter;
                        stream_filter << obj["filter"];
                        std::string str_filter = stream_filter.str();

                        std::stringstream stream_iops;
                        stream_iops << obj["period"]["iops"];
                        uint32_t iops{0};
                        stream_iops >> iops;

                        std::stringstream stream_bw;
                        stream_bw << obj["period"]["bw"];
                        uint64_t bw{0};
                        stream_bw >> bw;

                        std::string iopsMetricId;
                        std::string bwMetricId;

                        if (0 == str_filter.compare("\"AIR_READ\""))
                        {
                            iopsMetricId = TEL120001_READ_IOPS_PER_PORT;
                            bwMetricId = TEL120002_READ_RATE_BYTES_PER_SECOND_PER_PORT;
                        }
                        else
                        {
                            iopsMetricId = TEL120011_WRITE_IOPS_PER_PORT;
                            bwMetricId = TEL120012_WRITE_RATE_BYTES_PER_SECOND_PER_PORT;
                        }
                        AddPerformanceMetric(posMetricVector, iopsMetricId,
                            POSMetricTypes::MT_GAUGE, iops, obj, interval, PerformanceMetric::PORT);
                        AddPerformanceMetric(posMetricVector, bwMetricId,
                            POSMetricTypes::MT_GAUGE, bw, obj, interval, PerformanceMetric::PORT);
                    }
                }

                if (data.HasKey("LAT_ARR_VOL_READ"))
                {
                    auto& objs = data["LAT_ARR_VOL_READ"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_mean;
                        stream_mean << obj["period"]["mean"];
                        uint32_t mean{0};
                        stream_mean >> mean;

                        std::stringstream stream_max;
                        stream_max << obj["period"]["max"];
                        uint32_t max{0};
                        stream_max >> max;

                        AddLatencyMetric(posMetricVector, TEL50002_READ_LATENCY_MEAN_NS,
                            POSMetricTypes::MT_GAUGE, mean, obj, interval);
                        AddLatencyMetric(posMetricVector, TEL50003_READ_LATENCY_MAX_NS,
                            POSMetricTypes::MT_GAUGE, max, obj, interval);
                    }
                }

                if (data.HasKey("LAT_ARR_VOL_WRITE"))
                {
                    auto& objs = data["LAT_ARR_VOL_WRITE"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_mean;
                        stream_mean << obj["period"]["mean"];
                        uint32_t mean{0};
                        stream_mean >> mean;

                        std::stringstream stream_max;
                        stream_max << obj["period"]["max"];
                        uint32_t max{0};
                        stream_max >> max;

                        AddLatencyMetric(posMetricVector, TEL50012_WRITE_LATENCY_MEAN_NS,
                            POSMetricTypes::MT_GAUGE, mean, obj, interval);
                        AddLatencyMetric(posMetricVector, TEL50013_WRITE_LATENCY_MAX_NS,
                            POSMetricTypes::MT_GAUGE, max, obj, interval);
                    }
                }

                if (data.HasKey("PERF_SSD_Read"))
                {
                    auto& objs = data["PERF_SSD_Read"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_filter;
                        stream_filter << obj["filter"];
                        std::string str_filter = stream_filter.str();

                        std::stringstream stream_iops;
                        stream_iops << obj["period"]["iops"];
                        uint32_t iops{0};
                        stream_iops >> iops;

                        std::stringstream stream_bw;
                        stream_bw << obj["period"]["bw"];
                        uint64_t bw{0};
                        stream_bw >> bw;

                        std::string iopsMetricId;
                        std::string bwMetricId;

                        if (0 == str_filter.compare("\"AIR_UNKNOWN\""))
                        {
                            iopsMetricId = TEL20000_READ_UNKNOWN_IOPS_PER_SSD;
                            bwMetricId = TEL20006_READ_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_META\""))
                        {
                            iopsMetricId = TEL20001_READ_META_IOPS_PER_SSD;
                            bwMetricId = TEL20007_READ_META_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_GC\""))
                        {
                            iopsMetricId = TEL20002_READ_GC_IOPS_PER_SSD;
                            bwMetricId = TEL20008_READ_GC_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_HOST\""))
                        {
                            iopsMetricId = TEL20003_READ_HOST_IOPS_PER_SSD;
                            bwMetricId = TEL20009_READ_HOST_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_FLUSH\""))
                        {
                            iopsMetricId = TEL20004_READ_FLUSH_IOPS_PER_SSD;
                            bwMetricId = TEL20010_READ_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else
                        {
                            iopsMetricId = TEL20005_READ_REBUILD_IOPS_PER_SSD;
                            bwMetricId = TEL20011_READ_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        AddPerformanceMetric(posMetricVector, iopsMetricId,
                            POSMetricTypes::MT_GAUGE, iops, obj, interval, PerformanceMetric::GET_SSD_ID);
                        AddPerformanceMetric(posMetricVector, bwMetricId,
                            POSMetricTypes::MT_GAUGE, bw, obj, interval, PerformanceMetric::GET_SSD_ID);
                    }
                }

                if (data.HasKey("PERF_SSD_Write"))
                {
                    auto& objs = data["PERF_SSD_Write"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_filter;
                        stream_filter << obj["filter"];
                        std::string str_filter = stream_filter.str();

                        std::stringstream stream_iops;
                        stream_iops << obj["period"]["iops"];
                        uint32_t iops{0};
                        stream_iops >> iops;

                        std::stringstream stream_bw;
                        stream_bw << obj["period"]["bw"];
                        uint64_t bw{0};
                        stream_bw >> bw;

                        std::string iopsMetricId;
                        std::string bwMetricId;

                        if (0 == str_filter.compare("\"AIR_UNKNOWN\""))
                        {
                            iopsMetricId = TEL20012_WRITE_UNKNOWN_IOPS_PER_SSD;
                            bwMetricId = TEL20018_WRITE_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_META\""))
                        {
                            iopsMetricId = TEL20013_WRITE_META_IOPS_PER_SSD;
                            bwMetricId = TEL20019_WRITE_META_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_GC\""))
                        {
                            iopsMetricId = TEL20014_WRITE_GC_IOPS_PER_SSD;
                            bwMetricId = TEL20020_WRITE_GC_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_HOST\""))
                        {
                            iopsMetricId = TEL20015_WRITE_HOST_IOPS_PER_SSD;
                            bwMetricId = TEL20021_WRITE_HOST_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else if (0 == str_filter.compare("\"AIR_FLUSH\""))
                        {
                            iopsMetricId = TEL20016_WRITE_FLUSH_IOPS_PER_SSD;
                            bwMetricId = TEL20022_WRITE_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        else
                        {
                            iopsMetricId = TEL20017_WRITE_REBUILD_IOPS_PER_SSD;
                            bwMetricId = TEL20023_WRITE_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD;
                        }
                        AddPerformanceMetric(posMetricVector, iopsMetricId,
                            POSMetricTypes::MT_GAUGE, iops, obj, interval, PerformanceMetric::GET_SSD_ID);
                        AddPerformanceMetric(posMetricVector, bwMetricId,
                            POSMetricTypes::MT_GAUGE, bw, obj, interval, PerformanceMetric::GET_SSD_ID);
                    }
                }

                if (data.HasKey("UTIL_REACTOR"))
                {
                    auto& objs = data["UTIL_REACTOR"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_usage;
                        stream_usage << obj["period"]["usage"];
                        uint64_t usage{0};
                        stream_usage >> usage;

                        AddCommonMetric(posMetricVector, TEL70000_SPDK_REACTOR_UTILIZATION,
                            POSMetricTypes::MT_GAUGE, usage, obj, interval);
                    }
                }

                if (data.HasKey("CNT_PendingIO"))
                {
                    auto& objs = data["CNT_PendingIO"]["objs"];
                    for (auto& obj_it : objs)
                    {
                        auto& obj = objs[obj_it.first];

                        std::stringstream stream_count;
                        stream_count << obj["period"]["count"];
                        int64_t count{0};
                        stream_count >> count;

                        AddCommonMetric(posMetricVector, TEL80000_DEVICE_PENDING_IO_COUNT,
                            POSMetricTypes::MT_GAUGE, count, obj, interval);
                    }
                }

                this->PublishTimeTriggeredMetric(posMetricVector);

                if (!posMetricVector->empty())
                {
                    this->telPub->PublishMetricList(posMetricVector);
                }
                else
                {
                    delete posMetricVector;
                    posMetricVector = nullptr;
                }
            }
        }
        catch (std::exception& e)
        {
            POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::TELEMETRY_AIR_DATA_PARSING_FAILED),
                "TelemetryAirDelegator failed to parsing data : {}", e.what());
            this->returnState = State::ERR_DATA;

            posMetricVector->clear();
            delete posMetricVector;
            posMetricVector = nullptr;
        }
        return this->returnState;
    };
}

TelemetryAirDelegator::~TelemetryAirDelegator(void)
{
    if (uptimeMetricGenerator != nullptr)
    {
        delete uptimeMetricGenerator;
        uptimeMetricGenerator = nullptr;
    }
}

void
TelemetryAirDelegator::SetState(State state)
{
    const std::lock_guard<std::mutex> lock(mutex);
    returnState = state;
}

void
TelemetryAirDelegator::RegisterAirEvent(void)
{
    air_request_data(
        {"PERF_ARR_VOL", "PERF_PORT", "LAT_ARR_VOL_READ", "LAT_ARR_VOL_WRITE", "PERF_SSD_Read",
            "PERF_SSD_Write", "UTIL_REACTOR", "CNT_PendingIO"},
        std::move(dataHandler));
}

} // namespace pos
