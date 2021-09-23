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

#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
TelemetryPublisher::TelemetryPublisher(void)
: globalPublisher(nullptr),
  turnOn(false) // todo: change default true or using config
{
}

TelemetryPublisher::~TelemetryPublisher(void)
{
}

void
TelemetryPublisher::SetMaxEntryLimit(int limit)
{
    dataPool.SetMaxEntryLimit(limit);
}

int
TelemetryPublisher::GetNumEntries(void)
{
    return dataPool.GetNumEntries();
}

void
TelemetryPublisher::StartPublishing(void)
{
    turnOn = true;
}

void
TelemetryPublisher::StopPublishing(void)
{
    turnOn = false;
}

bool
TelemetryPublisher::IsRunning(void)
{
    return turnOn;
}

int
TelemetryPublisher::PublishData(std::string id, uint32_t value)
{
    if (turnOn == false)
    {
        return -1;
    }

    MetricUint32 metric;
    time_t time = std::time(nullptr);
    metric.SetMetric(id, time, value, _GetTimeString(time));

    dataPool.SetLog(metric);
    int ret = globalPublisher->PublishToServer(metric);

    return ret;
}

int
TelemetryPublisher::PublishData(std::string id, std::string value)
{
    if (turnOn == false)
    {
        return -1;
    }

    MetricString metric;
    time_t time = std::time(nullptr);
    metric.SetMetric(id, time, value, _GetTimeString(time));

    // string type log wouldn't be stored in client's data pool to minimize the memory usage
    // dataPool.SetLog(metric);
    int ret = globalPublisher->PublishToServer(metric);

    return ret;
}

int
TelemetryPublisher::CollectData(std::string id, MetricUint32& outLog)
{
    return dataPool.GetLog(id, outLog);
}

list<MetricUint32>
TelemetryPublisher::CollectAll(void)
{
    return dataPool.GetAll();
}

void
TelemetryPublisher::SetGlobalPublisher(IGlobalPublisher* gp)
{
    globalPublisher = gp;
}

std::string
TelemetryPublisher::_GetTimeString(time_t time)
{
    tm curTime = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&curTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace pos
