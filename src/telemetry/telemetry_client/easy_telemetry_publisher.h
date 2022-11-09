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
#include <tbb/concurrent_queue.h>

#include <atomic>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/lib/singleton.h"
#include "src/telemetry/telemetry_client/pos_metric.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class ConfigManager;
class MetaFsTimeInterval;

struct MetricWithFlags
{
    POSMetric metric;
    bool isUpdated;
};

using VectorLabels = std::vector<std::pair<std::string, std::string>>;
using MetricRepository = std::vector<std::unordered_map<size_t, MetricWithFlags>>;

class EasyTelemetryPublisher
{
public:
    explicit EasyTelemetryPublisher(TelemetryPublisher* tp = nullptr, MetaFsTimeInterval* interval = nullptr);
    virtual ~EasyTelemetryPublisher(void);

    virtual void Initialize(ConfigManager* config, const cpu_set_t& generalCpuSet);

    virtual void IncreaseCounter(const std::string& id);
    virtual void IncreaseCounter(const std::string& id, const VectorLabels& labels);
    virtual void IncreaseCounter(const std::string& id, const uint64_t value);
    virtual void IncreaseCounter(const std::string& id, const uint64_t value,
        const VectorLabels& labels);

    virtual void UpdateGauge(const std::string& id, const uint64_t value);
    virtual void UpdateGauge(const std::string& id, const uint64_t value,
        const VectorLabels& labels);

    /** only for test **/
    virtual void PublishMetricsWithLabels(void);
    virtual void UpdateMetrics(void);
    MetricRepository GetMetricRepository(void)
    {
        return metrics;
    }
    void RunWorker(void)
    {
        _RunWorker();
    }
    void StopWorker(void)
    {
        _StopWorker();
    }
    /** only for test **/

private:
    void _PeriodicPublish(void);
    void _RunWorker(void);
    void _StopWorker(void);
    void _UpdateInterval(ConfigManager* config);

    POSMetric _CreateGaugeMetric(const std::string& id, const uint64_t value,
        const VectorLabels& labels);
    POSMetric _CreateCounterMetric(const std::string& id, const uint64_t value,
        const VectorLabels& labels);

    const uint32_t DEFAULT_INTERVAL_IN_MILLISECOND;
    const std::string PUBLISHER_NAME;
    TelemetryPublisher* publisher;

    bool isCreatedPublisher;
    uint32_t interval_in_millisecond;

    tbb::concurrent_queue<POSMetric> queue;
    MetricRepository metrics;

    std::thread* worker;
    std::atomic<bool> isRunnable;

    MetaFsTimeInterval* timeInterval;
};

using EasyTelemetryPublisherSingleton = Singleton<EasyTelemetryPublisher>;

} // namespace pos
