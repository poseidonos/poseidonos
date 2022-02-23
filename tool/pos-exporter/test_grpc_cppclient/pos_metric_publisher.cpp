#include "pos_metric_publisher.h"

bool POSMetricPublisher::messageSend(MetricPublishRequest* request) {
    MetricPublishResponse response;
    grpc::ClientContext context;
    grpc::Status status;

    status = stub->MetricPublish(&context, *request, &response);
    if(!status.ok()) {
        return false;
    }
    return true;
}

void POSMetricPublisher::addLabelListIntoMetric(Metric* metric, const label_t& labelList) {
    for(auto& kv : labelList) {
        Label* label = metric->add_labels();
        label->set_key(kv.first);
        label->set_value(kv.second);
    }
}

void POSMetricPublisher::PublishCounter(std::string name, label_t labelList, uint64_t value) {
    MetricPublishRequest *request = new MetricPublishRequest{};
    Metric* metric = request->add_metrics();

    metric->set_name(name.c_str());
    metric->set_type(MetricTypes::COUNTER);
    addLabelListIntoMetric(metric, labelList);
    metric->set_countervalue(value);

    bool ret = messageSend(request);
    assert(ret);

    delete request;
}

void POSMetricPublisher::PublishGauge(std::string name, label_t labelList, int64_t value) {
    MetricPublishRequest *request = new MetricPublishRequest{};
    Metric* metric = request->add_metrics();

    metric->set_name(name.c_str());
    metric->set_type(MetricTypes::GAUGE);
    addLabelListIntoMetric(metric, labelList);
    metric->set_gaugevalue(value);

    bool ret = messageSend(request);
    assert(ret);

    delete request;
}

void POSMetricPublisher::PublishHistogram(std::string name, label_t labelList, std::vector<int64_t> upperBound, std::vector<uint64_t> bucketCount, int64_t sum, uint64_t totalCount) {
    MetricPublishRequest *request = new MetricPublishRequest{};
    Metric* metric = request->add_metrics();

    metric->set_name(name.c_str());
    metric->set_type(MetricTypes::HISTOGRAM);
    addLabelListIntoMetric(metric, labelList);

    // build histogram value
    HistogramValue *hValue = new HistogramValue;
    for(int64_t ub : upperBound) {
        hValue->add_bucketrange(ub);
    }
    for(uint64_t bc : bucketCount) {
        hValue->add_bucketcount(bc);
    }
    hValue->set_sum(sum);
    hValue->set_totalcount(totalCount);

    metric->set_allocated_histogramvalue(hValue);
    bool ret = messageSend(request);
    assert(ret);

    delete request;
   // delete hValue; // NOTE:: (set_allocated_histogram has ownership of hValue, so don't need to release after sending gRPC message)
}