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
#include "src/telemetry/telemetry_client/i_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_data_pool.h"
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <unordered_map>

#define DEFAULT_PUBLICATION_LIST_FILE_PATH "/etc/pos/publication_list_default.yaml"

namespace pos
{
class TelemetryPublisher
{
public:
    TelemetryPublisher(void) = default;
    explicit TelemetryPublisher(std::string name);
    virtual ~TelemetryPublisher(void);
    virtual void StartPublishing(void);
    virtual void StopPublishing(void);
    virtual bool IsRunning(void);
    virtual void StartUsingDataPool(void);
    virtual void StopUsingDataPool(void);

    virtual void SetMaxEntryLimit(int limit);
    void SetName(std::string name_);
    virtual std::string GetName(void);

    virtual int PublishData(std::string id_, POSMetricValue value_, POSMetricTypes type_);
    virtual int PublishMetric(POSMetric metric);
    virtual int PublishMetricList(std::vector<POSMetric>* metricList);
    virtual POSMetricVector* AllocatePOSMetricVector(void);
    virtual void SetGlobalPublisher(IGlobalPublisher* gp);
    virtual int AddDefaultLabel(std::string key, std::string value);
    void LoadPublicationList(std::string filePath);

private:
    const std::string PUBLICATION_LIST_ROOT = "metrics_to_publish";
    bool selectivePublication = false;
    std::string name;
    IGlobalPublisher* globalPublisher;
    TelemetryDataPool dataPool;
    bool turnOn;
    bool useDataPool;
    MetricLabelMap defaultlabelList;

    bool _ShouldPublish(std::string metricId);
    void _LoadPublicationList(std::string filePath);
    void _RemoveMetricNotToPublish(POSMetricVector* metricList);
    std::unordered_map<std::string, bool> publicationList;
};

} // namespace pos
