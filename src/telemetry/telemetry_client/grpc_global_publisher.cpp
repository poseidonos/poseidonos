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

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"

namespace pos
{
GrpcGlobalPublisher::GrpcGlobalPublisher(std::shared_ptr<grpc::Channel> channel_)
{
    std::string serverAddr = TEL_SERVER_IP; // TODO: temporary
    std::shared_ptr<grpc::Channel> channel = channel_;
    if (channel == nullptr)
    {
        channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials());
    }
    stub = ::MetricManager::NewStub(channel);
}

GrpcGlobalPublisher::~GrpcGlobalPublisher(void)
{
}

int
GrpcGlobalPublisher::PublishToServer(MetricLabelMap* defaultLabelList, POSMetricVector* metricList)
{
    int ret = 0;
    assert(metricList != nullptr);
    uint32_t cnt = 0;
    MetricPublishRequest* request = new MetricPublishRequest;
    for (auto& mit : (*metricList))
    {
        cnt++;
        if (cnt > MAX_NUM_METRICLIST)
        {
            POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[Telemetry] Failed to add MetricList, numMetric overflowed!!!, name:{}, numMetric:{}", mit.GetName(), metricList->size());
            break;
        }
        // set values
        Metric* metric = request->add_metrics();
        metric->set_name(mit.GetName());
        POSMetricTypes type = mit.GetType();
        if (type == MT_COUNT)
        {
            metric->set_type(MetricTypes::COUNTER);
            metric->set_countervalue(mit.GetCountValue());
        }
        else if (type == MT_GAUGE)
        {
            metric->set_type(MetricTypes::GAUGE);
            metric->set_gaugevalue(mit.GetGaugeValue());
        }
        else if (type == MT_HISTOGRAM)
        {
            // build gRPC histogram data structure
            HistogramValue *histValue = new HistogramValue;
            auto& mitHistUpperBound = mit.GetHistogramValue()->GetUpperBound();
            auto& mitHistBucketCount = mit.GetHistogramValue()->GetBucketCount();
            for (size_t i = 0; i < mitHistUpperBound.size(); i++)
            {
                histValue->add_bucketrange(mitHistUpperBound[i]);
                histValue->add_bucketcount(mitHistBucketCount[i]);
            }
            /**
             newly created HistogramValue will be removed by gRPC
             ref : https://developers.google.com/protocol-buffers/docs/reference/cpp-generated  , void set_allocated_foo(string* value
            */
            metric->set_type(MetricTypes::HISTOGRAM);
            metric->set_allocated_histogramvalue(histValue);
        }
        else
        {
            assert(false);
        }

        if (defaultLabelList != nullptr)
        {
            for (auto& dlit : (*defaultLabelList))
            {
                Label* lab = metric->add_labels();
                lab->set_key(dlit.first);
                lab->set_value(dlit.second);
            }
        }
        // set user label
        MetricLabelMap* labelList = mit.GetLabelList();
        for (auto& lit : (*labelList))
        {
            Label* lab = metric->add_labels();
            lab->set_key(lit.first);
            lab->set_value(lit.second);
        }
    }
    ret = _SendMessage(request, cnt);
    delete request;
    return ret;
}

int
GrpcGlobalPublisher::_SendMessage(MetricPublishRequest* request, uint32_t numMetrics)
{
    MetricPublishResponse response;
    grpc::ClientContext cliContext;
    grpc::Status status = stub->MetricPublish(&cliContext, *request, &response);
    if (status.ok() != true)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[TelemetryClient] Failed to send PublishRequest by gRPC, errorcode:{}, errormsg:{}", status.error_code(), status.error_message());
        return -1;
    }
    else
    {
        uint32_t numReceived = response.totalreceivedmetrics();
        if (numReceived != numMetrics)
        {
            POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[TelemetryClient] TelemetryManager responsed with error: received data count mismatch, expected:{}, received:{}", numMetrics, numReceived);
            return -1;
        }
    }
    return 0;
}

} // namespace pos
