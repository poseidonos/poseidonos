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
    // given 1.
    tp.StopPublishing();
    // when 1.
    int ret = tp.PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, 100);
    // then 1.
    EXPECT_EQ(-1, ret);
    // given 2.
    tp.StartPublishing();
    // when 2.
    ret = tp.PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, 200);
    // then 2.
    EXPECT_EQ(0, ret);
    // given 3.
    tp.PublishData(TEL001_ALCT_FREE_SEG_CNT, 10);
    tp.PublishData(TEL001_ALCT_FREE_SEG_CNT, 9);
    MetricUint32 log;
    // when 3.
    ret = tp.CollectData(TEL001_ALCT_FREE_SEG_CNT, log);
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

    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    // given 1.
    int ret = tp.PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, 100);
    ret = tp.PublishData(TEL001_ALCT_FREE_SEG_CNT, 200);
    // when 1.
    ret = tp.PublishData(TEL004_ALCT_GCMODE, 3);
    ret = tp.GetNumEntries();
    // then 1.
    EXPECT_EQ(2, ret);
    // when 2.
    ret = tp.PublishData(TEL001_ALCT_FREE_SEG_CNT, 300);
    // then 2.
    EXPECT_EQ(0, ret);
    MetricUint32 log;
    tp.CollectData(TEL001_ALCT_FREE_SEG_CNT, log);
    EXPECT_EQ(300, log.GetValue());
}

} // namespace pos
