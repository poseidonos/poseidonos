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
#include "src/telemetry/telemetry_client/pos_metric.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include <map>
#include <chrono>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace pos
{
POSMetric::POSMetric(std::string name_, POSMetricTypes type_)
{
    name = name_;
    type = type_;
    // time = std::time(nullptr);
}

void
POSMetric::SetName(std::string name_)
{
    name = name_;
}

void
POSMetric::SetType(POSMetricTypes type_)
{
    type = type_;
}

void
POSMetric::SetCountValue(uint64_t count_)
{
    type = MT_COUNT;
    value.count = count_;
}

void
POSMetric::SetGaugeValue(int64_t gauge_)
{
    type = MT_GAUGE;
    value.gauge = gauge_;
}

void
POSMetric::SetHistogramValue(POSHistogramValue* histogram_)
{
    type = MT_HISTOGRAM;
    value.histogram = histogram_;
}

int
POSMetric::AddLabel(std::string label, std::string key)
{
    if (labelList.size() == MAX_NUM_LABEL)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] Failed to add Label, numLabel is overflowed!!!!, label:{}, key:{}", label, key);
        return -1;
    }
    labelList.emplace(label, key);
    return 0;
}

std::string
POSMetric::GetName(void)
{
    return name;
}

POSMetricTypes
POSMetric::GetType(void)
{
    return type;
}

uint64_t
POSMetric::GetCountValue(void)
{
    return value.count;
}

int64_t
POSMetric::GetGaugeValue(void)
{
    return value.gauge;
}

POSHistogramValue*
POSMetric::GetHistogramValue(void)
{
    return value.histogram;
}

MetricLabelMap*
POSMetric::GetLabelList(void)
{
    return (&labelList);
}


POSHistogramValue::POSHistogramValue(std::vector<int64_t> upperBound_)
{
    size_t bucketSize = upperBound_.size();
    if (bucketSize == 0)
    {
        assert(false);
    }
    upperBound.resize(bucketSize);
    bucketCount.resize(bucketSize);
    sum = 0;
    totalCount = 0;

    for (size_t i = 0; i < bucketSize; i++)
    {
        upperBound[i] = upperBound_[i];
        bucketCount[i] = 0;
    }
}

void
POSHistogramValue::Observe(int64_t value)
{
    for(size_t i = 0; i < upperBound.size(); i++)
    {
        if (value <= upperBound[i])
        {
            bucketCount[i] += 1;
        }
    }
    sum = sum + value;
    totalCount += 1;
}

const std::vector<int64_t>&
POSHistogramValue::GetUpperBound(void) const
{
    return upperBound;
}

const std::vector<uint64_t>&
POSHistogramValue::GetBucketCount(void) const
{
    return bucketCount;
}

int64_t
POSHistogramValue::GetSum(void)
{
    return sum;
}

uint64_t
POSHistogramValue::GetTotalCount(void)
{
    return totalCount;
}

} // namespace pos
