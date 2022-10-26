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
    easyTp.PublishMetricsWithLabels();
}

TEST(EasyTelemetryPublisher, EnqueueMetric_testIfPublishedCorrectlyWhenCallerGivesSingleMetric)
{
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockConfigManager> config;
    cpu_set_t generalCpuSet;
    EasyTelemetryPublisher easyTp(&tp);
    easyTp.Initialize(&config, generalCpuSet);

    EXPECT_CALL(tp, PublishMetricList).WillOnce(Return(0));

    POSMetric m("test", POSMetricTypes::MT_GAUGE);
    easyTp.EnqueueMetric(m);
    easyTp.PublishMetricsWithLabels();
}
} // namespace pos
