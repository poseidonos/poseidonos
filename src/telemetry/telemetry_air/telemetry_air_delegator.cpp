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

namespace pos
{
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

    if (uptimeMetricGenerator != nullptr)
    {
        POSMetric uptimeMetric;
        int ret = uptimeMetricGenerator->Generate(&uptimeMetric);
        if (ret != -1 && posMetricVector != nullptr)
        {
            posMetricVector->push_back(uptimeMetric);
        }
    }
}

std::string
ToString(const air::JSONdoc& data)
{
    std::stringstream stream;
    std::string result;
    stream << data;
    stream >> result;
    if ('"' == result[0])
    {
        return result.substr(1, result.size() - 2);
    }
    else
    {
        return result;
    }
}

void
SetMetricValue(POSMetric* metric, const air::JSONdoc& airNodeObj, const AirMetricInfo& info)
{
    std::string data_category{"cumulation"};
    if (info.airPeriod)
    {
        data_category = "period";
    }

    if (POSMetricTypes::MT_COUNT == info.metricType)
    {
        uint64_t count_value{ToPrimitive<uint64_t>(airNodeObj[data_category][info.airValue])};
        metric->SetCountValue(count_value);
    }
    else if (POSMetricTypes::MT_GAUGE == info.metricType)
    {
        int64_t gauge_value{ToPrimitive<int64_t>(airNodeObj[data_category][info.airValue])};
        metric->SetGaugeValue(gauge_value);
    }
}

void
SetCommonLabel(POSMetric* metric, const air::JSONdoc& airNodeObj)
{
    metric->AddLabel("thread_id", ToString(airNodeObj["target_id"]));
    metric->AddLabel("thread_name", ToString(airNodeObj["target_name"]));
    metric->AddLabel("index", ToString(airNodeObj["index"]));
    metric->AddLabel("filter", ToString(airNodeObj["filter"]));
}

void
SetCustomArrayIdVolumeIdLabel(POSMetric* metric, const air::JSONdoc& airNodeObj)
{
    uint64_t index{ToPrimitive<uint64_t>(airNodeObj["index"])};
    uint64_t arrayId{(index & 0xFF00) >> 8};
    uint64_t volumeId{index & 0x00FF};
    metric->AddLabel("array_id", std::to_string(arrayId));
    metric->AddLabel("volume_id", std::to_string(volumeId));
}

void
SetCustomSourceDeviceIdLabel(POSMetric* metric, const air::JSONdoc& airNodeObj)
{
    metric->AddLabel("device_id", ToString(airNodeObj["index"]));
    metric->AddLabel("source", ToString(airNodeObj["filter"]));
}

void
SetCustomPortNumberLabel(POSMetric* metric, const air::JSONdoc& airNodeObj)
{
    uint64_t index{ToPrimitive<uint64_t>(airNodeObj["index"])};
    std::string portId[4], port;
    portId[0] = std::to_string(index & 0xFF);
    portId[1] = std::to_string((index & 0xFF00) >> 8);
    portId[2] = std::to_string((index & 0xFF0000) >> 16);
    portId[3] = std::to_string((index & 0xFF000000) >> 24);
    port = std::to_string(index >> 32);
    metric->AddLabel("port", portId[0] + "." + portId[1] + "." + portId[2] + "." + portId[3] + ":" + port);
}

void
SetCustomLabel(POSMetric* metric, const air::JSONdoc& airNodeObj, const AirMetricInfo& info)
{
    switch (info.customLabel)
    {
        case (CustomLabel::ArrayIdVolumeId):
            SetCustomArrayIdVolumeIdLabel(metric, airNodeObj);
            break;
        case (CustomLabel::SourceDeviceId):
            SetCustomSourceDeviceIdLabel(metric, airNodeObj);
            break;
        case (CustomLabel::PortNumber):
            SetCustomPortNumberLabel(metric, airNodeObj);
            break;
        case (CustomLabel::None):
        default:
            break;
    }
}

void
PrintMetric(POSMetric& metric)
{
    std::cout << metric.GetName() << ", ";
    if (POSMetricTypes::MT_COUNT == metric.GetType())
    {
        std::cout << "count value: " << metric.GetCountValue() << ", ";
    }
    else if (POSMetricTypes::MT_GAUGE == metric.GetType())
    {
        std::cout << "gauge value: " << metric.GetGaugeValue() << ", [ ";
    }
    auto label_list{metric.GetLabelList()};
    for (auto& label_iter : *label_list)
    {
        std::cout << "(" << label_iter.first << ":" << label_iter.second << ") ";
    }
    std::cout << "]\n";
}

POSMetric*
MakeMetric(const air::JSONdoc& airNodeObj, const AirMetricInfo& info)
{
    if (0 == info.airFilter.compare(ToString(airNodeObj["filter"])) || info.airFilter.empty())
    {
        POSMetric* posMetric{new POSMetric(info.telemetryID, info.metricType)};
        SetMetricValue(posMetric, airNodeObj, info);
        SetCommonLabel(posMetric, airNodeObj);
        SetCustomLabel(posMetric, airNodeObj, info);
        return posMetric;
    }
    return nullptr;
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
                    interval = ToString(data["interval"]);
                }

                for (auto& airMetricInfo : this->airMetricInfoList)
                {
                    if (data.HasKey(airMetricInfo.airNodeName))
                    {
                        auto& objs = data[airMetricInfo.airNodeName]["objs"];
                        for (auto& obj_it : objs)
                        {
                            auto& obj = objs[obj_it.first];
                            auto metric{MakeMetric(obj, airMetricInfo)};
                            if (nullptr != metric)
                            {
                                metric->AddLabel("interval", interval);
                                posMetricVector->push_back(*metric);
                                delete metric;
                                metric = nullptr;
                            }
                        }
                    }
                }

                this->PublishTimeTriggeredMetric(posMetricVector);

                if (!posMetricVector->empty())
                {
                    /*
                    for (auto& v : *posMetricVector)
                    {
                        PrintMetric(v);
                    }
                    */
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
            POS_TRACE_ERROR(static_cast<int>(EID(TELEMETRY_AIR_DATA_PARSING_FAILED)),
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
            "PERF_SSD_Write", "CNT_PendingIO", "VolumeIo_Constructor", "VolumeIo_Destructor",
            "Ubio_Constructor", "Ubio_Destructor", "SSD_Submit", "SSD_Complete", "EventQueue_Push",
            "WorkerCommonQueue_Push", "WorkerCommonQueue_Pop", "Callback_Constructor", "Callback_Destructor",
            "Event_Constructor", "Event_Destructor", "IOWorker_Submit", "IOWorker_Complete",
            "RequestedUserRead", "RequestedUserWrite", "RequestedUserAdminIo",
            "CompleteUserRead", "CompleteUserWrite", "CompleteUserAdminIo",
            "UserFlushProcess", "PartialWriteProcess", "UserFailIo",
            "UserReadPendingCnt", "UserWritePendingCnt", "InternalIoPendingCnt", "TimeOutIoCnt"},
        std::move(dataHandler));
}

} // namespace pos
