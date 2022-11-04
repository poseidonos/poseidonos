#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <set>
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
        usleep(1);
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
        usleep(1);
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
        usleep(1);
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
        usleep(1);
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
        usleep(1);
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
        usleep(1);
    }

    {
        vector<POSMetric> metrics = fakePublisher->GetMetrics();
        ASSERT_EQ(metrics.size(), expectedSize);
        EXPECT_EQ(metrics[0].GetName(), metricId);
        EXPECT_EQ(metrics[0].GetGaugeValue(), expectedLastGauge);
    }

    publisher->StopWorker();
}

TEST_F(EasyTelemetryPublisherTest, testIfMetricsWithVariousLabelsArePublishedAsIntended)
{
    const string metricId_1 = "test_metric_1";
    const string metricId_2 = "test_metric_2";
    const size_t expectedCounter = 1;
    const size_t expectedSize = 4;

    // #1
    {
        VectorLabels label;
        label.push_back({"label1", "val1"});
        label.push_back({"label2", "val2"});
        publisher->IncreaseCounter(metricId_1, label);
    }

    // #2
    {
        VectorLabels label;
        label.push_back({"label1", "val1"});
        label.push_back({"label2", "val3"});
        publisher->IncreaseCounter(metricId_1, label);
    }

    // #3
    {
        VectorLabels label;
        label.push_back({"label2", "val1"});
        label.push_back({"label3", "val2"});
        publisher->IncreaseCounter(metricId_1, label);
    }

    // #1
    {
        VectorLabels label;
        label.push_back({"label2", "val2"});
        label.push_back({"label1", "val1"});
        publisher->IncreaseCounter(metricId_1, label);
    }

    // #4
    {
        VectorLabels label;
        label.push_back({"label2", "val2"});
        label.push_back({"label1", "val1"});
        fakePublisher->SetUpdated(false);
        publisher->IncreaseCounter(metricId_2, label);
    }

    publisher->RunWorker();

    // wait for update
    while(!fakePublisher->IsUpdated())
    {
        usleep(1);
    }

    vector<POSMetric> metrics = fakePublisher->GetMetrics();
    ASSERT_EQ(metrics.size(), expectedSize);

    set<string> ids;
    for (auto& metric : metrics)
    {
        ids.insert(metric.GetName());
        if (!(metric.GetName() == metricId_1 || metric.GetName() == metricId_2))
        {
            ASSERT_TRUE(false) << "not expected name: " << metric.GetName();
        }
    }
    EXPECT_EQ(ids.size(), 2);

    set<uint64_t> values;
    for (auto& metric : metrics)
    {
        values.insert(metric.GetCountValue());
        if (!(1 <= metric.GetCountValue() || metric.GetCountValue() >= 2))
        {
            ASSERT_TRUE(false) << "not expected value: " << metric.GetCountValue();
        }
    }
    EXPECT_EQ(values.size(), 2);

    publisher->StopWorker();
}
} // namespace pos
