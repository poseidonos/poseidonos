#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "test/unit-tests/metafs/lib/metafs_time_interval_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

using namespace std;

namespace pos
{
class FakeTelemetryPublisher : public TelemetryPublisher
{
public:
    explicit FakeTelemetryPublisher(std::string name)
    : TelemetryPublisher(name),
      isUpdated(false)
    {
    }

    virtual ~FakeTelemetryPublisher(void)
    {
    }

    virtual int PublishMetricList(std::vector<POSMetric>* metricList) override
    {
        metrics.clear();
        for (auto& metric : *metricList)
        {
            metrics.push_back(metric);
        }
        delete metricList;
        isUpdated = true;
        return true;
    }

    vector<POSMetric> GetMetrics(void)
    {
        return metrics;
    }

    bool IsUpdated(void)
    {
        return isUpdated;
    }

    void SetUpdated(bool flag)
    {
        isUpdated = flag;
    }

private:
    vector<POSMetric> metrics;
    atomic<bool> isUpdated;
};

class EasyTelemetryPublisherTest : public ::testing::Test
{
public:
    EasyTelemetryPublisherTest(void)
    : publisher(nullptr)
    {
        _SetCpuCore();
    }

    virtual ~EasyTelemetryPublisherTest(void)
    {
    }

    virtual void SetUp(void)
    {
        fakePublisher = new FakeTelemetryPublisher("test");
        // EasyTelemetryPublisher will destruct
        interval = new NiceMock<MockMetaFsTimeInterval>(0);
        ON_CALL(*interval, CheckInterval).WillByDefault(Return(true));
        config = new NiceMock<MockConfigManager>;

        publisher = new EasyTelemetryPublisher(fakePublisher, interval);
        publisher->Initialize(config, cpuCore);
    }

    virtual void TearDown(void)
    {
        delete publisher;
        delete fakePublisher;
        delete config;
    }

protected:
    void _SetCpuCore(void)
    {
        CPU_SET(0, &cpuCore);
    }

    EasyTelemetryPublisher* publisher;
    FakeTelemetryPublisher* fakePublisher;
    NiceMock<MockMetaFsTimeInterval>* interval;
    NiceMock<MockConfigManager>* config;

    cpu_set_t cpuCore;
};

TEST_F(EasyTelemetryPublisherTest, testIfThePublisherPublishesSingleCounterMetric)
{
    const string metricId = "test_metric";
    const size_t expectedCounter = 1;
    publisher->IncreaseCounter(metricId);
    publisher->RunWorker();

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
    }

    vector<POSMetric> metrics = fakePublisher->GetMetrics();
    const size_t expectedSize = 1;
    ASSERT_EQ(metrics.size(), expectedSize);
    EXPECT_EQ(metrics[0].GetName(), metricId);
    EXPECT_EQ(metrics[0].GetCountValue(), expectedCounter);

    publisher->StopWorker();
}

TEST_F(EasyTelemetryPublisherTest, testIfThePublisherPublishesSingleGaugeMetric)
{
    const string metricId = "test_metric";
    const size_t expectedGauge = 1;
    publisher->UpdateGauge(metricId, expectedGauge);
    publisher->RunWorker();

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
    }

    vector<POSMetric> metrics = fakePublisher->GetMetrics();
    const size_t expectedSize = 1;
    ASSERT_EQ(metrics.size(), expectedSize);
    EXPECT_EQ(metrics[0].GetName(), metricId);
    EXPECT_EQ(metrics[0].GetGaugeValue(), expectedGauge);

    publisher->StopWorker();
}

TEST_F(EasyTelemetryPublisherTest, testIfThePublisherPublishesSingleCounterMetricTwice)
{
    const string metricId = "test_metric";
    const size_t expectedFirstCounter = 1;
    const size_t expectedLastCounter = 2;
    const size_t expectedSize = 1;

    publisher->IncreaseCounter(metricId);
    publisher->RunWorker();

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
    }

    {
        vector<POSMetric> metrics = fakePublisher->GetMetrics();
        ASSERT_EQ(metrics.size(), expectedSize);
        EXPECT_EQ(metrics[0].GetName(), metricId);
        EXPECT_EQ(metrics[0].GetCountValue(), expectedFirstCounter);
    }

    fakePublisher->SetUpdated(false);
    publisher->IncreaseCounter(metricId);

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
    }

    {
        vector<POSMetric> metrics = fakePublisher->GetMetrics();
        ASSERT_EQ(metrics.size(), expectedSize);
        EXPECT_EQ(metrics[0].GetName(), metricId);
        EXPECT_EQ(metrics[0].GetCountValue(), expectedLastCounter);
    }

    publisher->StopWorker();
}

TEST_F(EasyTelemetryPublisherTest, testIfThePublisherPublishesSingleGaugeMetricTwice)
{
    const string metricId = "test_metric";
    const size_t expectedFirstGauge = 10;
    const size_t expectedLastGauge = 20;
    const size_t expectedSize = 1;

    publisher->UpdateGauge(metricId, expectedFirstGauge);
    publisher->RunWorker();

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
    }

    {
        vector<POSMetric> metrics = fakePublisher->GetMetrics();
        ASSERT_EQ(metrics.size(), expectedSize);
        EXPECT_EQ(metrics[0].GetName(), metricId);
        EXPECT_EQ(metrics[0].GetGaugeValue(), expectedFirstGauge);
    }

    fakePublisher->SetUpdated(false);
    publisher->UpdateGauge(metricId, expectedLastGauge);

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
    }

    {
        vector<POSMetric> metrics = fakePublisher->GetMetrics();
        ASSERT_EQ(metrics.size(), expectedSize);
        EXPECT_EQ(metrics[0].GetName(), metricId);
        EXPECT_EQ(metrics[0].GetGaugeValue(), expectedLastGauge);
    }

    publisher->StopWorker();
}
} // namespace pos
