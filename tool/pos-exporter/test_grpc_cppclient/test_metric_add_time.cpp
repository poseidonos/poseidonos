#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include <grpc++/grpc++.h>
#include "metric/metric.grpc.pb.h"

using label_t = std::vector<std::pair<std::string,std::string>>;

class POSMetricPublisher {
public:
    POSMetricPublisher(std::shared_ptr<grpc::Channel> channel) {
        stub = MetricManager::NewStub(channel);
    }

    void PublishCounter(std::string name, label_t labelList, uint64_t value);
    void PublishGauge(std::string name, label_t labelList, int64_t value);
    void PublishHistogram(std::string name, label_t labelList, int64_t upperBound, uint64_t bucketCount, int64_t sum, uint64_t totalCount);

private:
    void addLabelListIntoMetric(Metric* metric, const label_t& labelList);
    bool messageSend(MetricPublishRequest* request);
    std::unique_ptr<MetricManager::Stub> stub;
};

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

    messageSend(request);

    delete request;
}

void POSMetricPublisher::PublishGauge(std::string name, label_t labelList, int64_t value) {
    MetricPublishRequest *request = new MetricPublishRequest{};
    Metric* metric = request->add_metrics();

    metric->set_name(name.c_str());
    metric->set_type(MetricTypes::GAUGE);
    addLabelListIntoMetric(metric, labelList);
    metric->set_gaugevalue(value);

    messageSend(request);
}

void POSMetricPublisher::PublishHistogram(std::string name, label_t labelList, int64_t upperBound, uint64_t bucketCount, int64_t sum, uint64_t totalCount) {
    MetricPublishRequest *request = new MetricPublishRequest{};
    Metric* metric = request->add_metrics();

    metric->set_name(name.c_str());
    metric->set_type(MetricTypes::HISTOGRAM);
    addLabelListIntoMetric(metric, labelList);
}

static inline std::chrono::system_clock::time_point GetCurrentTime() {
    return std::chrono::system_clock::now();
}

static int64_t GetElapsedTimeUS(std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end) {
    std::chrono::microseconds mil = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    return mil.count();
}

static void SummaryTimeLog(std::string prefix, std::vector<int64_t>& timeLog) {
    int64_t sum = 0;
    int64_t max = INT_MIN;
    int64_t min = 1;
    for(auto timeEntry : timeLog) {
        sum = sum + timeEntry;
        max = std::max(max, timeEntry);
        min = std::min(min, timeEntry);
    }

    printf("Test[%s] Avg=%.2f(us) Max=%lld(us) Min=%lld(us) \n", prefix.c_str(), sum/static_cast<double>(timeLog.size()), max, min);
}


static void Test_MeasureSingleMetric() {
    const int NUM_RUN = 10000;
    POSMetricPublisher pub(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    label_t labelList;

    /* generate fixed label list */
    labelList.push_back(std::pair<std::string,std::string>{"label1","label1_value"});
    labelList.push_back(std::pair<std::string,std::string>{"label2","label2_value"});

    std::vector<int64_t> timeLog;
    timeLog.resize(NUM_RUN);
    for(int i = 0; i < NUM_RUN; i++) {
        auto s = GetCurrentTime();
        pub.PublishCounter("Test1_Counter", labelList, i);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Counter 1000 Items", timeLog);

}


int main(int argc, char* argv[]) {

    Test_MeasureSingleMetric();

    return 0;
}