#include "src/telemetry/telemetry_client/telemetry_publisher.h"

#include <gtest/gtest.h>

#include "test/unit-tests/telemetry/telemetry_client/i_global_publisher_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(TelemetryPublisher, PublishData_TestUpdateData)
{
    // given
    TelemetryPublisher tp;
    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    tp.StartUsingDataPool();
    // given 1.
    tp.StopPublishing();
    // when 1.
    POSMetric m("a", MT_GAUGE);
    m.SetCountValue(1);
    m.SetGaugeValue(2);
    m.AddLabel("1", "tt");
    m.AddLabel("2", "cc");
    time_t t;
    m.SetTime(t);
    m.SetType(MT_GAUGE);
    t = m.GetTime();
    POSMetricTypes tt = m.GetType();
    std::string temp = m.GetName();
    uint64_t cnt = m.GetCountValue();
    int64_t gg = m.GetGaugeValue();

    POSMetricValue v;
    v.gauge = 1;
    // given 1.
    int ret = tp.PublishData("cc", v, MT_GAUGE);
    // given 2.
    tp.StartPublishing();
    // when 2.
    ret = tp.PublishData("cc", v, MT_GAUGE);
}

TEST(TelemetryPublisher, PublishData_TestExceedEntryLimit)
{
    // given
    TelemetryPublisher tp;
    tp.SetMaxEntryLimit(2);
    tp.StartPublishing();
    tp.StartUsingDataPool();

    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    POSMetric m("a", MT_GAUGE);
    m.SetCountValue(1);
    m.SetGaugeValue(2);
    m.AddLabel("1", "tt");
    m.AddLabel("2", "cc");
    time_t t;
    m.SetTime(t);
    m.SetType(MT_GAUGE);
    t = m.GetTime();
    POSMetricTypes tt = m.GetType();
    std::string temp = m.GetName();
    uint64_t cnt = m.GetCountValue();
    int64_t gg = m.GetGaugeValue();

    POSMetricValue v;
    v.gauge = 1;
    // given 1.
    int ret = tp.PublishData("cc", v, MT_GAUGE);
    v.count = 0;
    // when 1.
    ret = tp.PublishData("cc", v, MT_COUNT);
    // when 2.
    ret = tp.PublishMetric(m);
    // when 3.
    std::vector<POSMetric>* list = tp.AllocatePOSMetricVector();
    list->push_back(m);
    m.SetName("aaa");
    list->push_back(m);
    // when 4.
    // ret = tp.PublishDataList(list); // TODO: Activate after MetricManager applied to TelemetryManager Server
    delete list;
    // then 2.
    // EXPECT_EQ(0, ret);

    // USAGE SAMPLE FOR PUBLISH METRIC LIST
    POSMetricVector* metricList = tp.AllocatePOSMetricVector();
    for (int i = 0; i < 5; i++)
    {
        std::string name = "key_" + to_string(i);
        POSMetric metric(name, MT_COUNT);
        metric.SetCountValue(i);
        metric.AddLabel("label0", "k1"); // optional
        metric.AddLabel("label1", "k2"); // optional
        metricList->push_back(metric);
        // tp.PublishDataList(metricList); // TODO: Activate after MetricManager applied to TelemetryManager Server
    }
    ////////////////////////////////////////
    tp.StopUsingDataPool();
    delete metricList;
    delete igp;
}

} // namespace pos
