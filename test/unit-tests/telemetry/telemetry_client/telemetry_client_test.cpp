#include "src/telemetry/telemetry_client/telemetry_client.h"

#include <gtest/gtest.h>

#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(TelemetryClient, RegisterClient_TestRegisterDeregister)
{
    // given
    NiceMock<TelemetryPublisher>* tc = new NiceMock<TelemetryPublisher>();
    TelemetryClient telClient;
    // when 1.
    int ret = telClient.RegisterPublisher("a", tc);
    // then 2.
    EXPECT_EQ(0, ret);
    // when 2.
    ret = telClient.RegisterPublisher("a", tc);
    // then 2.
    EXPECT_EQ(-1, ret);
    // given 3.
    telClient.DeregisterPublisher("a");
    telClient.DeregisterPublisher("a");
    // when 3.
    ret = telClient.RegisterPublisher("a", tc);
    // then 3.
    EXPECT_EQ(0, ret);
    delete tc;
}

TEST(TelemetryClient, StartPublisher_TestStartAndStop)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher();
    TelemetryClient telClient;
    telClient.RegisterPublisher("a", tp);
    // when 1.
    telClient.StartPublisher("a");
    // then 1.
    bool ret = telClient.IsPublisherRunning("a");
    EXPECT_EQ(true, ret);
    // when 2.
    telClient.StopPublisher("a");
    // then 2.
    ret = telClient.IsPublisherRunning("a");
    EXPECT_EQ(false, ret);
    delete tp;
}

TEST(TelemetryClient, StartTelemetryPublisherAll_TestStartAndStopAll)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher();
    TelemetryPublisher* tp2 = new TelemetryPublisher();
    TelemetryClient telClient;
    telClient.RegisterPublisher("a", tp);
    telClient.RegisterPublisher("b", tp2);
    // when 1.
    telClient.StartAllPublisher();
    // then 1.
    bool ret = telClient.IsPublisherRunning("a");
    EXPECT_EQ(true, ret);
    ret = telClient.IsPublisherRunning("b");
    EXPECT_EQ(true, ret);
    // when 2.
    telClient.StopAllPublisher();
    // then 2.
    ret = telClient.IsPublisherRunning("a");
    EXPECT_EQ(false, ret);
    ret = telClient.IsPublisherRunning("b");
    EXPECT_EQ(false, ret);
    delete tp;
    delete tp2;
}

TEST(TelemetryClient, CollectValue_TestData)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher();
    TelemetryClient telClient;
    telClient.RegisterPublisher("a", tp);
    telClient.StartPublisher("a");
    tp->PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 10);
    TelemetryGeneralMetric log;
    // when 1.
    int ret = telClient.CollectValue("a", TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, log);
    // then 1.
    EXPECT_EQ(0, ret);
    EXPECT_EQ(10, log.GetValue());
    // when 2.
    ret = telClient.CollectValue("b", TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, log);
    // then 2.
    EXPECT_EQ(-1, ret);
    // when 3.
    ret = telClient.CollectValue("a", "noname", log);
    // then 3.
    EXPECT_EQ(-1, ret);
    delete tp;
}

TEST(TelemetryClient, CollectList_TestData)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher();
    TelemetryClient telClient;
    telClient.RegisterPublisher("a", tp);
    telClient.StartPublisher("a");

    tp->PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 10);
    tp->PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 20);
    tp->PublishData(TEL_ALLOCATOR_GCVICTIM_SEGMENT, 30);
    tp->PublishData(TEL_ALLOCATOR_GCMODE, 1);
    // when
    list<TelemetryGeneralMetric> retList = telClient.CollectList("a");
    // then
    for (auto &p : retList)
    {
        if (p.GetId() == TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT)
        {
            EXPECT_EQ(10, p.GetValue());
        }
        else if (p.GetId() == TEL_ALLOCATOR_FREE_SEGMENT_COUNT)
        {
            EXPECT_EQ(20, p.GetValue());
        }
        else if (p.GetId() == TEL_ALLOCATOR_GCVICTIM_SEGMENT)
        {
            EXPECT_EQ(30, p.GetValue());
        }
        else if (p.GetId() == TEL_ALLOCATOR_GCMODE)
        {
            EXPECT_EQ(1, p.GetValue());
        }
    }
    delete tp;
}

} // namespace pos
