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

#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"

#include <sched.h>

#include <thread>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/metafs/lib/metafs_time_interval.h"

namespace pos
{
EasyTelemetryPublisher::EasyTelemetryPublisher(TelemetryPublisher* tp, MetaFsTimeInterval* interval)
: DEFAULT_INTERVAL_IN_MILLISECOND(1000),
  PUBLISHER_NAME("EasyTelemetryPublisher"),
  publisher(tp),
  isCreatedPublisher(false),
  interval_in_millisecond(DEFAULT_INTERVAL_IN_MILLISECOND),
  worker(nullptr),
  isRunnable(false),
  timeInterval(interval)
{
    for (int i = 0; i < (int)POSMetricTypes::MT_NUM_TYPE; ++i)
    {
        metrics.push_back(std::unordered_map<size_t, MetricWithFlags>());
    }

    if (!publisher)
    {
        isCreatedPublisher = true;
        publisher = new TelemetryPublisher(PUBLISHER_NAME);
        TelemetryClientSingleton::Instance()->RegisterPublisher(publisher);
    }

    POS_TRACE_INFO(EID(SUCCESS), "Easy Telemetry Publisher has been created");
}

EasyTelemetryPublisher::~EasyTelemetryPublisher(void)
{
    if (isCreatedPublisher)
    {
        isCreatedPublisher = false;
        _StopWorker();

        TelemetryClientSingleton::Instance()->DeregisterPublisher(PUBLISHER_NAME);
        delete publisher;
    }

    if (timeInterval)
    {
        delete timeInterval;
    }

    POS_TRACE_INFO(EID(SUCCESS), "Easy Telemetry Publisher has been deleted");
}

void
EasyTelemetryPublisher::Initialize(ConfigManager* config, const cpu_set_t& generalCpuSet)
{
    if (isCreatedPublisher)
    {
        assert(config != nullptr);
        sched_setaffinity(0, sizeof(generalCpuSet), &generalCpuSet);
        _UpdateInterval(config);
        _RunWorker();
    }

    if (!timeInterval)
    {
        timeInterval = new MetaFsTimeInterval(interval_in_millisecond);
    }

    POS_TRACE_INFO(EID(SUCCESS),
        "Easy Telemetry Publisher is to periodically publish stats, coreId:{}",
        sched_getcpu());
}

void
EasyTelemetryPublisher::IncreaseCounter(const std::string& id)
{
    IncreaseCounter(id, 1);
}

void
EasyTelemetryPublisher::IncreaseCounter(const std::string& id, const VectorLabels& labels)
{
    IncreaseCounter(id, 1, labels);
}

void
EasyTelemetryPublisher::IncreaseCounter(const std::string& id, const uint64_t value)
{
    VectorLabels emptyLabels;
    IncreaseCounter(id, value, emptyLabels);
}

void
EasyTelemetryPublisher::IncreaseCounter(const std::string& id, const uint64_t value,
    const VectorLabels& labels)
{
    queue.push(_CreateCounterMetric(id, value, labels));
}

void
EasyTelemetryPublisher::UpdateGauge(const std::string& id, const uint64_t value)
{
    VectorLabels emptyLabels;
    UpdateGauge(id, value, emptyLabels);
}

void
EasyTelemetryPublisher::UpdateGauge(const std::string& id, const uint64_t value,
    const VectorLabels& labels)
{
    queue.push(_CreateGaugeMetric(id, value, labels));
}

POSMetric
EasyTelemetryPublisher::_CreateGaugeMetric(const std::string& id, const uint64_t value,
    const VectorLabels& labels)
{
    POSMetric m(id, POSMetricTypes::MT_GAUGE);
    m.SetGaugeValue(value);
    for (auto& label : labels)
    {
        m.AddLabel(label.first, label.second);
    }

    return m;
}

POSMetric
EasyTelemetryPublisher::_CreateCounterMetric(const std::string& id, const uint64_t value,
    const VectorLabels& labels)
{
    POSMetric m(id, POSMetricTypes::MT_COUNT);
    m.SetCountValue(value);
    for (auto& label : labels)
    {
        m.AddLabel(label.first, label.second);
    }

    return m;
}

void
EasyTelemetryPublisher::_RunWorker(void)
{
    POS_TRACE_INFO(EID(SUCCESS), "Easy Telemetry Publisher is going to create a thread to publish metrics");
    assert(worker == nullptr);
    isRunnable = true;
    worker = new std::thread(std::bind(&EasyTelemetryPublisher::_PeriodicPublish, this));
}

void
EasyTelemetryPublisher::_StopWorker(void)
{
    POS_TRACE_INFO(EID(SUCCESS), "Easy Telemetry Publisher is going to make the worker stop");
    if (worker != nullptr)
    {
        isRunnable = false;
        worker->join();
        POS_TRACE_INFO(EID(SUCCESS), "The worker has been stopped");
    }
    else
    {
        POS_TRACE_WARN(EID(TELEMETRY_THREAD_IS_NOT_EXIST), "There is no thread to stop");
    }
}

void
EasyTelemetryPublisher::_UpdateInterval(ConfigManager* config)
{
    uint32_t interval = 0;
    std::string module = "telemetry";
    std::string key = "interval_in_millisecond_for_easy_telemetry_publisher";
    std::string additionalStr = "using default value";

    if (!config->GetValue(module, key, &interval, ConfigType::CONFIG_TYPE_UINT32))
    {
        interval_in_millisecond = interval;
        additionalStr = "as loaded";
    }
    else
    {
        POS_TRACE_WARN(EID(TELEMETRY_INTERVAL_HAS_NOT_BEEN_LOADED), "The interval has not been loaded");
    }

    POS_TRACE_INFO(EID(SUCCESS), "Easy Telemetry Publisher is going to publish metrics every {} ms {}",
        interval_in_millisecond, additionalStr);
}

void
EasyTelemetryPublisher::_PeriodicPublish(void)
{
    while (isRunnable)
    {
        UpdateMetrics();

        if (timeInterval && timeInterval->CheckInterval())
        {
            PublishMetricsWithLabels();
        }

        usleep(1);
    }

    POS_TRACE_INFO(EID(SUCCESS), "The worker has been stopped");
}

void
EasyTelemetryPublisher::UpdateMetrics(void)
{
    POSMetric metric;
    while (queue.try_pop(metric))
    {
        MetricWithFlags m{metric, true};
        auto hashed = m.metric.Hash();
        auto& map = metrics[(int)metric.GetType()];

        auto itor = map.find(hashed);
        if (itor == map.end())
        {
            // new entry and/or label
            switch (metric.GetType())
            {
                case POSMetricTypes::MT_COUNT:
                    m.metric.SetCountValue(metric.GetCountValue());
                    break;
                case POSMetricTypes::MT_GAUGE:
                    m.metric.SetGaugeValue(metric.GetGaugeValue());
                    break;
                default:
                    POS_TRACE_WARN(EID(TELEMETRY_NOT_SUPPORT_TYPE),
                        "The metric type {} is not supported", (int)metric.GetType());
                    assert(false);
                    break;
            }
            map.insert({hashed, m});
        }
        else
        {
            // existing entry
            switch (metric.GetType())
            {
                case POSMetricTypes::MT_COUNT:
                {
                    auto newValue = itor->second.metric.GetCountValue() + metric.GetCountValue();
                    itor->second.metric.SetCountValue(newValue);
                }
                break;
                case POSMetricTypes::MT_GAUGE:
                    itor->second.metric.SetGaugeValue(metric.GetGaugeValue());
                    break;
                default:
                    assert(false);
                    break;
            }
            itor->second.isUpdated = true;
        }
    }
}

void
EasyTelemetryPublisher::PublishMetricsWithLabels(void)
{
    std::vector<POSMetric>* m = new std::vector<POSMetric>;
    for (auto& map : metrics)
    {
        for (auto& metric : map)
        {
            if (metric.second.isUpdated)
            {
                metric.second.isUpdated = false;
                m->push_back(metric.second.metric);
            }
        }
    }

    if (m->size() > 0)
    {
        publisher->PublishMetricList(m);
    }
    else
    {
        delete m;
    }
}

} // namespace pos
