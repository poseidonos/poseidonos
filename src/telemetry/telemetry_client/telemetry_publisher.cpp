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

#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/master_context/instance_id_provider.h"

namespace pos
{
TelemetryPublisher::TelemetryPublisher(std::string name_)
: globalPublisher(nullptr),
  turnOn(false),
  useDataPool(false) // todo: change default true or using config
{
    name = name_;
    defaultlabelList.emplace(TEL_PUBNAME_LABEL_KEY, name);
    std::string runId = to_string(InstanceIdProviderSingleton::Instance()->GetInstanceId());
    defaultlabelList.emplace(TEL_RUNID_LABEL_KEY, runId);
    
    _LoadPublicationList(DEFAULT_PUBLICATION_LIST_FILE_PATH);
}

TelemetryPublisher::~TelemetryPublisher(void)
{
}

void
TelemetryPublisher::SetMaxEntryLimit(int limit)
{
    dataPool.SetMaxEntryLimit(limit);
}

std::string
TelemetryPublisher::GetName(void)
{
    return name;
}

void
TelemetryPublisher::SetName(std::string name_)
{
    name = name_;
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

void
TelemetryPublisher::StartUsingDataPool(void)
{
    useDataPool = true;
}

void
TelemetryPublisher::StopUsingDataPool(void)
{
    useDataPool = false;
}

POSMetricVector*
TelemetryPublisher::AllocatePOSMetricVector(void)
{
    POSMetricVector* ret = new POSMetricVector();
    return ret;
}

int
TelemetryPublisher::PublishData(std::string id, POSMetricValue value, POSMetricTypes type)
{
    if ((turnOn == false) || (_ShouldPublish(id) == false))
    {
        return -1;
    }

    POSMetric metric(id, type);
    if (type == MT_COUNT)
    {
        metric.SetCountValue(value.count);
    }
    else if (type == MT_GAUGE)
    {
        metric.SetGaugeValue(value.gauge);
    }
    else if (type == MT_HISTOGRAM)
    {
        metric.SetHistogramValue(value.histogram);
    }
    POSMetricVector* metricList = AllocatePOSMetricVector();
    metricList->push_back(metric);
    int ret = globalPublisher->PublishToServer(&defaultlabelList, metricList);
    metricList->clear();
    delete metricList;
    return ret;
}

int
TelemetryPublisher::PublishMetric(POSMetric metric)
{
    if ((turnOn == false) || (_ShouldPublish(metric.GetName()) == false))
    {
        return -1;
    }

    POSMetricVector* metricList = AllocatePOSMetricVector();
    metricList->push_back(metric);
    int ret = globalPublisher->PublishToServer(&defaultlabelList, metricList);
    metricList->clear();
    delete metricList;
    return ret;
}

int
TelemetryPublisher::PublishMetricList(POSMetricVector* metricList)
{
    if (turnOn == false)
    {
        metricList->clear();
        delete metricList;
        return -1;
    }
    _RemoveMetricNotToPublish(metricList);

    if (metricList->size() <= 0)
    {
        return -1;
    }

    int ret = globalPublisher->PublishToServer(&defaultlabelList, metricList);
    metricList->clear();
    delete metricList;
    return ret;
}

void
TelemetryPublisher::SetGlobalPublisher(IGlobalPublisher* gp)
{
    globalPublisher = gp;
}

int
TelemetryPublisher::AddDefaultLabel(std::string key, std::string value)
{
    if (defaultlabelList.size() == MAX_NUM_LABEL)
    {
        POS_TRACE_WARN(EID(TELEMETRY_PUBLISHER_LABEL_ADD_FAILURE_MAXNUM_VIOLATION),
            "key:{}, value:{}, defaultlabellist_size:{}, max_num_label:{}", 
            key, value, defaultlabelList.size(), MAX_NUM_LABEL);
        return -1;
    }
    defaultlabelList.emplace(key, value);
    return 0;
}

void
TelemetryPublisher::LoadPublicationList(std::string filePath)
{
    _LoadPublicationList(filePath);
}

void
TelemetryPublisher::_LoadPublicationList(std::string filePath)
{
    YAML::Node list;

    try
    {
        list = YAML::LoadFile(filePath)[PUBLICATION_LIST_ROOT];

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            std::string tag = it->first.as<std::string>();
            std::pair<std::string, bool> metricToPublish (tag, true);            
            publicationList.insert(metricToPublish);
        }

        POS_TRACE_INFO(EID(TELEMETRY_PUBLISHER_PUBLICATION_LIST_LOAD_SUCCESS),
            "filePath:{}, list_size:{}", filePath, publicationList.size());
    }
    catch (YAML::Exception& e)
    {
        POS_TRACE_WARN(EID(TELEMETRY_PUBLISHER_PUBLICATION_LIST_LOAD_FAILURE),
            "filePath:{}, yaml_exception:{}", filePath, e.msg);
        selectivePublication = false;
    }
}

void
TelemetryPublisher::_RemoveMetricNotToPublish(POSMetricVector* metricList)
{
    auto it = metricList->begin();
    while(it != metricList->end())
    {
        if (_ShouldPublish(it->GetName()) == false)
        {
            it = metricList->erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool
TelemetryPublisher::_ShouldPublish(std::string metricId)
{
    if (selectivePublication == false)
    {
        return true;
    }

    return (publicationList.find(metricId) != publicationList.end());
}

} // namespace pos
