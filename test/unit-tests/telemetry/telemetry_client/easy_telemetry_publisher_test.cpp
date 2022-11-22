#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(EasyTelemetryPublisher, IncreaseCounter_testIfPublishedCorrectlyWhenCallerGivesOnlyId)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.IncreaseCounter(id);
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, IncreaseCounter_testIfPublishedCorrectlyWhenCallerGivesIdAndValue)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.IncreaseCounter(id, 1);
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, IncreaseCounter_testIfPublishedCorrectlyWhenCallerGivesIdAndLabel)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.IncreaseCounter(id, VectorLabels{{"test", "1"}, {"test2", "2"}});
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, IncreaseCounter_testIfPublishedCorrectlyWhenCallerGivesIdAndValueAndLabel)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.IncreaseCounter(id, 1, VectorLabels{{"test", "1"}, {"test2", "2"}});
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, IncreaseCounter_testIfNotPublishedWhenTheMetricIsAlreadyPublished)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.IncreaseCounter(id);
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();

    EXPECT_CALL(tp, PublishMetricList).Times(0);

    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, UpdateGauge_testIfPublishedCorrectlyWhenCallerGivesIdAndValue)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.UpdateGauge(id, 1);
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, UpdateGauge_testIfPublishedCorrectlyWhenCallerGivesIdAndValueAndLabel)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    std::string id = "test";
    easyTp.UpdateGauge(id, 1, VectorLabels{{"test", "1"}, {"test2", "2"}});
    easyTp.UpdateMetrics();
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, GetMetricRepository_testIfThePublisherHasExpectedMetric)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    // id: test1, label: {label1, val1}
    VectorLabels label1;
    label1.push_back({"label1", "val1"});
    easyTp.UpdateGauge("test1", 1, label1);

    // calc expected metric
    POSMetric expectedMetric("test1", POSMetricTypes::MT_GAUGE);
    expectedMetric.AddLabel("label1", "val1");
    size_t expectedHash = expectedMetric.Hash();

    // update metrics
    easyTp.UpdateMetrics();

    // get metrics
    MetricRepository repo = easyTp.GetMetricRepository();

    // id: test1, label: {label1, val1}
    ASSERT_NE(repo[(int)POSMetricTypes::MT_GAUGE].find(expectedHash), repo[(int)POSMetricTypes::MT_GAUGE].end());
    auto& metric = repo[(int)POSMetricTypes::MT_GAUGE][expectedHash];
    EXPECT_EQ(metric.metric.GetName(), "test1");
    EXPECT_EQ(metric.metric.GetType(), POSMetricTypes::MT_GAUGE);
    EXPECT_EQ(metric.metric.GetGaugeValue(), 1);
    EXPECT_EQ((*metric.metric.GetLabelList())["label1"], "val1");
}

TEST(EasyTelemetryPublisher, GetMetricRepository_testIfThePublisherHasExpectedMetricEvenIfTheMetricIsOverwrited)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    // id: test1, label: {label1, val1}
    VectorLabels label1;
    label1.push_back({"label1", "val1"});
    easyTp.UpdateGauge("test1", 1, label1);

    // id: test1, label: {label1, val1}
    // update value
    easyTp.UpdateGauge("test1", 2, label1);

    // calc expected metric
    POSMetric expectedMetric("test1", POSMetricTypes::MT_GAUGE);
    expectedMetric.AddLabel("label1", "val1");
    size_t expectedHash = expectedMetric.Hash();

    // update metrics
    easyTp.UpdateMetrics();

    // get metrics
    MetricRepository repo = easyTp.GetMetricRepository();

    // id: test1, label: {label1, val1}
    ASSERT_NE(repo[(int)POSMetricTypes::MT_GAUGE].find(expectedHash), repo[(int)POSMetricTypes::MT_GAUGE].end());
    auto& metric = repo[(int)POSMetricTypes::MT_GAUGE][expectedHash];
    EXPECT_EQ(metric.metric.GetName(), "test1");
    EXPECT_EQ(metric.metric.GetType(), POSMetricTypes::MT_GAUGE);
    EXPECT_EQ(metric.metric.GetGaugeValue(), 2);
    EXPECT_EQ((*metric.metric.GetLabelList())["label1"], "val1");
}

TEST(EasyTelemetryPublisher, IncreaseCounter_testIfTheMetricHasSameLabelsButNotTheSameSequence)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    // id: test1, label: {label1, 1}, {label2, 2}
    const string metricId = "test1";
    const size_t expectedCounter = 1;
    VectorLabels label;
    // label1 -> label2
    label.push_back({"label1", "1"});
    label.push_back({"label2", "2"});
    easyTp.IncreaseCounter(metricId, label);

    // calc expected metric
    POSMetric expectedMetric("test1", POSMetricTypes::MT_COUNT);
    expectedMetric.AddLabel("label1", "1");
    expectedMetric.AddLabel("label2", "2");
    size_t expectedHash = expectedMetric.Hash();

    // update metrics
    easyTp.UpdateMetrics();

    {
        // get metrics
        MetricRepository repo = easyTp.GetMetricRepository();

        // check count
        ASSERT_NE(repo[(int)POSMetricTypes::MT_COUNT].find(expectedHash), repo[(int)POSMetricTypes::MT_COUNT].end());
        ASSERT_EQ(repo[(int)POSMetricTypes::MT_COUNT].size(), expectedCounter);

        auto& metric = repo[(int)POSMetricTypes::MT_COUNT][expectedHash];
        EXPECT_EQ(metric.metric.GetName(), metricId);
        EXPECT_EQ(metric.metric.GetType(), POSMetricTypes::MT_COUNT);
        EXPECT_EQ(metric.metric.GetCountValue(), 1);
        EXPECT_EQ((*metric.metric.GetLabelList())["label1"], "1");
        EXPECT_EQ((*metric.metric.GetLabelList())["label2"], "2");
    }

    // change the sequence of the labels
    {
        VectorLabels label;
        // label2 -> label1
        label.push_back({"label2", "2"});
        label.push_back({"label1", "1"});
        easyTp.IncreaseCounter(metricId, label);
    }

    // update metrics
    easyTp.UpdateMetrics();

    {
        // get metrics
        MetricRepository repo = easyTp.GetMetricRepository();

        // check count
        ASSERT_NE(repo[(int)POSMetricTypes::MT_COUNT].find(expectedHash), repo[(int)POSMetricTypes::MT_COUNT].end());
        ASSERT_EQ(repo[(int)POSMetricTypes::MT_COUNT].size(), expectedCounter);

        auto& metric = repo[(int)POSMetricTypes::MT_COUNT][expectedHash];
        EXPECT_EQ(metric.metric.GetName(), metricId);
        EXPECT_EQ(metric.metric.GetType(), POSMetricTypes::MT_COUNT);
        EXPECT_EQ(metric.metric.GetCountValue(), 1 + 1);
        EXPECT_EQ((*metric.metric.GetLabelList())["label1"], "1");
        EXPECT_EQ((*metric.metric.GetLabelList())["label2"], "2");
    }
}

TEST(EasyTelemetryPublisher, GetMetricRepository_testIfThePublisherHasTwoExpectedMetricsEvenIfTheyHaveTheSameName)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    // id: test1, label: {label1, val1}
    VectorLabels label1;
    label1.push_back({"label1", "val1"});
    easyTp.UpdateGauge("test1", 1, label1);

    // id: test1, label: {label1, val1}
    VectorLabels label2;
    label2.push_back({"label1", "val2"});
    easyTp.UpdateGauge("test1", 10, label2);

    // calc expected metric1
    POSMetric expectedMetric("test1", POSMetricTypes::MT_GAUGE);
    expectedMetric.AddLabel("label1", "val1");
    size_t expectedHash = expectedMetric.Hash();

    // calc expected metric2
    POSMetric expectedMetric2("test1", POSMetricTypes::MT_GAUGE);
    expectedMetric2.AddLabel("label1", "val2");
    size_t expectedHash2 = expectedMetric2.Hash();

    // update metrics
    easyTp.UpdateMetrics();

    // get metrics
    MetricRepository repo = easyTp.GetMetricRepository();

    // check if it has two metrics
    ASSERT_NE(repo[(int)POSMetricTypes::MT_GAUGE].find(expectedHash), repo[(int)POSMetricTypes::MT_GAUGE].end());
    ASSERT_NE(repo[(int)POSMetricTypes::MT_GAUGE].find(expectedHash2), repo[(int)POSMetricTypes::MT_GAUGE].end());

    // id: test1, label: {label1, val1}
    auto& metric = repo[(int)POSMetricTypes::MT_GAUGE][expectedHash];
    EXPECT_EQ(metric.metric.GetName(), "test1");
    EXPECT_EQ(metric.metric.GetType(), POSMetricTypes::MT_GAUGE);
    EXPECT_EQ(metric.metric.GetGaugeValue(), 1);
    EXPECT_EQ((*metric.metric.GetLabelList())["label1"], "val1");

    // id: test1, label: {label1, val2}
    auto& metric2 = repo[(int)POSMetricTypes::MT_GAUGE][expectedHash2];
    EXPECT_EQ(metric2.metric.GetName(), "test1");
    EXPECT_EQ(metric2.metric.GetType(), POSMetricTypes::MT_GAUGE);
    EXPECT_EQ(metric2.metric.GetGaugeValue(), 10);
    EXPECT_EQ((*metric2.metric.GetLabelList())["label1"], "val2");
}
} // namespace pos
