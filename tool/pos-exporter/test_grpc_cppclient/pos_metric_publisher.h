#pragma once

#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <assert.h>

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
    void PublishHistogram(std::string name, label_t labelList, std::vector<int64_t> upperBound, std::vector<uint64_t> bucketCount, int64_t sum, uint64_t totalCount);

private:
    void addLabelListIntoMetric(Metric* metric, const label_t& labelList);
    bool messageSend(MetricPublishRequest* request);
    std::unique_ptr<MetricManager::Stub> stub;
};

