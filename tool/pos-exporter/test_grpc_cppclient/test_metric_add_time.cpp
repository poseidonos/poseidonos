#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <thread>

#include "pos_metric_publisher.h"



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

    printf("Test[%s, Items:%u] Total:%ld(ms) Avg=%.2f(us) Max=%ld(us) Min=%ld(us) \n", 
        prefix.c_str(), 
        (unsigned int) timeLog.size(), sum/1000, sum/static_cast<double>(timeLog.size()), max, min);
}


static void Test_MeasureSingleMetric() {
    const int NUM_RUN = 10000;
    POSMetricPublisher pub(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    label_t labelList;

    /* generate fixed label list */
    labelList.push_back(std::pair<std::string,std::string>{"label1","label1_value"});
    labelList.push_back(std::pair<std::string,std::string>{"label2","label2_value"});

    /* fixed upperBound, BucketCount for histogram */
    std::vector<int64_t> upperBound{0,10,20,30};
    std::vector<uint64_t> bucketCount{1,2,3,4};
    int64_t bucketSum = 60; // 0+10+20+30
    uint64_t bucketTotalCount = 4;

    std::vector<int64_t> timeLog;
    timeLog.resize(NUM_RUN);
    for(int i = 0; i < NUM_RUN; i++) {
        auto s = GetCurrentTime();
        pub.PublishCounter("Test1_Counter", labelList, i);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Counter Metric(Fixed Label)", timeLog);

    timeLog.clear();
    for(int i = 0; i < NUM_RUN; i++) {
        auto s = GetCurrentTime();
        pub.PublishGauge("Test1_Gauge", labelList, i);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Gauge Metric(Fixed Label)", timeLog);


    timeLog.clear();
    for(int i = 0; i < NUM_RUN; i++) {
        auto s = GetCurrentTime();
        pub.PublishHistogram("Test1_Histogram", labelList, upperBound, bucketCount, bucketSum, bucketTotalCount);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Histogram Metric(Fixed Label)", timeLog);
}


static void Test_MeasureSingleMetric_DynamicLabel() {
    const int NUM_RUN = 10000;
    POSMetricPublisher pub(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    label_t labelList;

    /* fixed upperBound, BucketCount for histogram */
    std::vector<int64_t> upperBound{0,10,20,30};
    std::vector<uint64_t> bucketCount{1,2,3,4};
    int64_t bucketSum = 60; // 0+10+20+30
    uint64_t bucketTotalCount = 4;

    std::vector<int64_t> timeLog;
    timeLog.resize(NUM_RUN);
    for(int i = 0; i < NUM_RUN; i++) {

        int labelCount = (std::rand() % 10) + 1;
        labelList.clear();
        for(int j = 0; j < labelCount; j++) {
            labelList.push_back(std::make_pair("label"+std::to_string(j), "label_v"+std::to_string(j)));
        }

        auto s = GetCurrentTime();
        pub.PublishCounter("Test1_Counter_DL"+std::to_string(i), labelList, i);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Counter Metric(Dynamic Label)", timeLog);

    timeLog.clear();
    for(int i = 0; i < NUM_RUN; i++) {
        int labelCount = (std::rand() % 10) + 1;
        labelList.clear();
        for(int j = 0; j < labelCount; j++) {
            labelList.push_back(std::make_pair("label"+std::to_string(j), "label_v"+std::to_string(j)));
        }

        auto s = GetCurrentTime();
        pub.PublishGauge("Test1_Gauge_DL"+std::to_string(i), labelList, i);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Gauge Metric(Dynamic Label)", timeLog);


    timeLog.clear();
    for(int i = 0; i < NUM_RUN; i++) {
        int labelCount = (std::rand() % 10) + 1;
        labelList.clear();
        for(int j = 0; j < labelCount; j++) {
            labelList.push_back(std::make_pair("label"+std::to_string(j), "label_v"+std::to_string(j)));
        }

        auto s = GetCurrentTime();
        pub.PublishHistogram("Test1_Histogram_DL"+std::to_string(i), labelList, upperBound, bucketCount, bucketSum, bucketTotalCount);
        timeLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
    }
    SummaryTimeLog("Histogram Metric(Dynamic Label)", timeLog);
}


static void Test_MeasureConcurrentMetric_DynamicLabel() {
    const int NUM_RUN = 10000;
    POSMetricPublisher pub(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::vector<int64_t> timeLogCounter;
    std::vector<int64_t> timeLogGauge;
    std::vector<int64_t> timeLogHistogram;

    timeLogCounter.resize(NUM_RUN);
    timeLogGauge.resize(NUM_RUN);
    timeLogHistogram.resize(NUM_RUN);

    printf("==========================================\n");
    printf("* Perform add multiple metric types simultaneously \n");
    printf("==========================================\n");

    std::thread counterThread([](int nRun, std::vector<int64_t> &tLog, POSMetricPublisher *pPub) {
        label_t labelList;
        printf(" ** (thread) add counter type metric \n");
        for(int i = 0; i < nRun; i++) {
            int labelCount = (std::rand() % 10) + 1;
            labelList.clear();
            for(int j = 0; j < labelCount; j++) {
                labelList.push_back(std::make_pair("label"+std::to_string(j), "label_v"+std::to_string(j)));
            }

            auto s = GetCurrentTime();
            pPub->PublishCounter("Test1_Counter_MT_DL"+std::to_string(i), labelList, i);
            tLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
        }
    }, NUM_RUN, std::ref(timeLogCounter), &pub);
    
    

    std::thread gaugeThread([](int nRun, std::vector<int64_t> &tLog, POSMetricPublisher *pPub) {
        label_t labelList;
        printf(" ** (thread) add gauge type metric \n");
        for(int i = 0; i < nRun; i++) {
            int labelCount = (std::rand() % 10) + 1;
            labelList.clear();
            for(int j = 0; j < labelCount; j++) {
                labelList.push_back(std::make_pair("label"+std::to_string(j), "label_v"+std::to_string(j)));
            }

            auto s = GetCurrentTime();
            pPub->PublishGauge("Test1_Gauge_MT_DL"+std::to_string(i), labelList, i);
            tLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
        }
    }, NUM_RUN, std::ref(timeLogGauge), &pub);
    
    

    std::thread histogramThread([](int nRun, std::vector<int64_t> &tLog, POSMetricPublisher *pPub) {
        /* fixed upperBound, BucketCount for histogram */
        std::vector<int64_t> upperBound{0,10,20,30};
        std::vector<uint64_t> bucketCount{1,2,3,4};
        int64_t bucketSum = 60; // 0+10+20+30
        uint64_t bucketTotalCount = 4;
        label_t labelList;

        printf(" ** (thread) add histogram type metric \n");
        for(int i = 0; i < nRun; i++) {
            int labelCount = (std::rand() % 10) + 1;
            labelList.clear();
            for(int j = 0; j < labelCount; j++) {
                labelList.push_back(std::make_pair("label"+std::to_string(j), "label_v"+std::to_string(j)));
            }

            auto s = GetCurrentTime();
            pPub->PublishHistogram("Test1_Histogram_MT_DL"+std::to_string(i), labelList, upperBound, bucketCount, bucketSum, bucketTotalCount);
            tLog.push_back(GetElapsedTimeUS(s, GetCurrentTime()));
        }
    }, NUM_RUN, std::ref(timeLogHistogram), &pub);

    counterThread.join();
    gaugeThread.join();
    histogramThread.join();

    SummaryTimeLog("Gauge Metric(Concurrent + Dynamic Label)", timeLogGauge);
    SummaryTimeLog("Gauge Metric(Concurrent + Dynamic Label)", timeLogGauge);
    SummaryTimeLog("Histogram Metric(Concurrent + Dynamic Label)", timeLogHistogram);
}


int main(int argc, char* argv[]) {

    Test_MeasureSingleMetric();
    Test_MeasureSingleMetric_DynamicLabel();
    Test_MeasureConcurrentMetric_DynamicLabel();

    return 0;
}