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

#include <list>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "proto/generated/cpp/metric.grpc.pb.h"
#include "proto/generated/cpp/metric.pb.h"
#include "src/helper/json/json_helper.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#define MAX_SAVE_METRIC_COUNT (100000)
#define NOT_FOUND (-1)

namespace pos
{

class TelemetryManagerService final : public MetricManager::Service
{
typedef struct
{
    int idx;
    int type;
    uint64_t counterValue;
    int64_t guageValue;
    time_t time;
    bool needUpdate;
}LoggedMetricValue;

public:
    TelemetryManagerService(void);
    virtual ~TelemetryManagerService(void);

    // Publisher -> Manager
    virtual ::grpc::Status MetricPublish(::grpc::ServerContext* context, const ::MetricPublishRequest* request, ::MetricPublishResponse* response) override;
    // Collector -> Manager
    virtual ::grpc::Status MetricCollect(::grpc::ServerContext* context, const ::MetricCollectRequest* request, ::MetricCollectResponse* response) override;

    void CreateTelemetryServer(std::string address);

    bool StartService(void);
    bool StopService(void);

    std::string GetServerAddr(void);

protected:
    std::thread* telemetryManagerServerThread;
    std::unique_ptr<::grpc::Server> server;

private:
    void _IncreaseMetricListHead(void);
    int _FindMetric(string metricName);

    bool enabledService;
    std::string serverAddress;

    int curMetricListHead;
    std::unordered_map<std::string, int> metricMap;

    MetricPublishRequest* metricList;
};
using TelemetryManagerServiceSingletone = Singleton<TelemetryManagerService>;
} // namespace pos
