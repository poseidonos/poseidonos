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

#include "space_info_publisher.h"

namespace pos
{
SpaceInfoPublisher::~SpaceInfoPublisher()
{
    if (publisher != nullptr)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(publisher->GetName());
    }
    delete publisher;
}

void
SpaceInfoPublisher::Register(uint32_t arrayId)
{
    _RegisterTelemetry(arrayId);
}

void
SpaceInfoPublisher::PublishTotalCapacity(uint32_t arrayId, uint64_t totalSize)
{
    POSMetric metric(TEL60004_ARRAY_CAPACITY_TOTAL, POSMetricTypes::MT_GAUGE);
    metric.SetGaugeValue(totalSize);
    metric.AddLabel("array_id", to_string(arrayId));
    publisher->PublishMetric(metric);
}

void
SpaceInfoPublisher::PublishUsedCapacity(uint32_t arrayId, uint64_t usedSize)
{
    POSMetric metric(TEL60005_ARRAY_CAPACITY_USED, POSMetricTypes::MT_GAUGE);
    metric.SetGaugeValue(usedSize);
    metric.AddLabel("array_id", to_string(arrayId));
    publisher->PublishMetric(metric);
}

void
SpaceInfoPublisher::_RegisterTelemetry(uint32_t arrayId)
{
    if (publisher == nullptr)
    {
        publisher = new TelemetryPublisher("SpaceInfo");
        publisher->AddDefaultLabel("array_id", to_string(arrayId));
        TelemetryClientSingleton::Instance()->RegisterPublisher(publisher);
    }
}

} // namespace pos
