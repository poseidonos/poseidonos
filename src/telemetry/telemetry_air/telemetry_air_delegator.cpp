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

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/pos_metric.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/telemetry/telemetry_id.h"

namespace pos
{
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

void
AddPerformanceMetric(POSMetricVector* posMetricVector,
    const std::string& name, const POSMetricTypes type, const uint64_t value,
    const air::JSONdoc& data, const std::string& interval, bool getSSDId = false)
{
    POSMetric posMetric{name, type};
    if (POSMetricTypes::MT_GAUGE)
    {
        posMetric.SetGaugeValue(value);
    }
    if(getSSDId)
    {
        AddSSDIdInterval(posMetric, data, interval);
    }
    else
    {
        AddLabelArrayVolumeInterval(posMetric, data, interval);
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
    if (POSMetricTypes::MT_GAUGE)
    {
        posMetric.SetGaugeValue(value);
    }

    AddLabelArrayVolumeInterval(posMetric, data, interval);

    std::stringstream stream_sample_count;
    stream_sample_count << data["period"]["sample_cnt"];
    posMetric.AddLabel("sample_count", stream_sample_count.str());

    posMetricVector->push_back(posMetric);
}

TelemetryAirDelegator::TelemetryAirDelegator(TelemetryPublisher* telPub)
: telPub(telPub)
{
    dataHandler = [this](const air::JSONdoc& data) -> int {
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

                        if (0 == str_filter.compare("\"AIR_READ\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL50000_READ_IOPS,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval);
                            AddPerformanceMetric(posMetricVector, TEL50001_READ_RATE_BYTES_PER_SECOND,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval);
                        }
                        else
                        {
                            AddPerformanceMetric(posMetricVector, TEL50010_WRITE_IOPS,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval);
                            AddPerformanceMetric(posMetricVector, TEL50011_WRITE_RATE_BYTES_PER_SECOND,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval);
                        }
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
                    bool getSSDId = true;
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

                        if (0 == str_filter.compare("\"AIR_UNKNOWN\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20000_READ_UNKNOWN_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20006_READ_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_META\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20001_READ_META_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20007_READ_META_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_GC\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20002_READ_GC_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20008_READ_GC_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_HOST\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20003_READ_HOST_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20009_READ_HOST_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_FLUSH\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20004_READ_FLUSH_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20010_READ_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else
                        {
                            AddPerformanceMetric(posMetricVector, TEL20005_READ_REBUILD_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20011_READ_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                    }
                }

                if (data.HasKey("PERF_SSD_Write"))
                {
                    auto& objs = data["PERF_SSD_Write"]["objs"];
                    bool getSSDId = true;
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

                        if (0 == str_filter.compare("\"AIR_UNKNOWN\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20012_WRITE_UNKNOWN_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20018_WRITE_UNKNOWN_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_META\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20013_WRITE_META_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20019_WRITE_META_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_GC\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20014_WRITE_GC_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20020_WRITE_GC_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_HOST\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20015_WRITE_HOST_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20021_WRITE_HOST_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else if (0 == str_filter.compare("\"AIR_FLUSH\""))
                        {
                            AddPerformanceMetric(posMetricVector, TEL20016_WRITE_FLUSH_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20022_WRITE_FLUSH_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                        else
                        {
                            AddPerformanceMetric(posMetricVector, TEL20017_WRITE_REBUILD_IOPS_PER_SSD,
                                POSMetricTypes::MT_GAUGE, iops, obj, interval, getSSDId);
                            AddPerformanceMetric(posMetricVector, TEL20023_WRITE_REBUILD_RATE_BYTES_PER_SECOND_PER_SSD,
                                POSMetricTypes::MT_GAUGE, bw, obj, interval, getSSDId);
                        }
                    }
                }

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
    air_request_data({"PERF_ARR_VOL", "LAT_ARR_VOL_READ", "LAT_ARR_VOL_WRITE", "PERF_SSD_Read", "PERF_SSD_Write"}, std::move(dataHandler));
}

} // namespace pos
