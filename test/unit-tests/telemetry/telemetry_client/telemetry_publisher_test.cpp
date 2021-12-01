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
TEST(TelemetryPublisher, PublishData_TestUpdateAndCollectItem)
{
    // given
    TelemetryPublisher tp;
    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    tp.StartUsingDataPool();
    // given 1.
    tp.StopPublishing();
    // when 1.
    int ret = tp.PublishData("2", 100);
    // then 1.
    EXPECT_EQ(-1, ret);
    // given 2.
    tp.StartPublishing();
    // when 2.
    ret = tp.PublishData("2", 200);
    // then 2.
    EXPECT_EQ(0, ret);
    // given 3.
    tp.PublishData("1", 10);
    tp.PublishData("1", 9);
    MetricUint32 log;
    // when 3.
    ret = tp.CollectData("1", log);
    // then 3.
    EXPECT_EQ(9, log.GetValue());
    // when 4.
    ret = tp.CollectData("ccc", log);
    // then 4.
    EXPECT_EQ(-1, ret);
}

TEST(TelemetryPublisher, PublishData_TestStringMetric)
{
    // given
    TelemetryPublisher tp;
    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    // given.
    tp.StartPublishing();
    std::string v;
    v = "hello world";
    // when
    int ret = tp.PublishData("string_id", v);
    // then.
    EXPECT_EQ(0, ret);
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
    // given 1.
    int ret = tp.PublishData("2", 100);
    ret = tp.PublishData("1", 200);
    // when 1.
    ret = tp.PublishData("4", 3);
    ret = tp.GetNumEntries();
    // then 1.
    EXPECT_EQ(2, ret);
    // when 2.
    ret = tp.PublishData("1", 300);
    // then 2.
    EXPECT_EQ(0, ret);
    MetricUint32 log;
    tp.CollectData("1", log);
    EXPECT_EQ(300, log.GetValue());
    tp.StopUsingDataPool();
}

} // namespace pos
