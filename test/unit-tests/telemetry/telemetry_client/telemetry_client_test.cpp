#include "src/telemetry/telemetry_client/telemetry_client.h"

#include <gtest/gtest.h>

#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
TEST(TelemetryClient, RegisterClient_TestRegisterDeregister)
{
    // given
    TelemetryPublisher* tc = new TelemetryPublisher();
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
    tp->StartUsingDataPool();
    tp->PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, 10);
    MetricUint32 log;
    // when 1.
    int ret = telClient.CollectValue("a", TEL002_ALCT_ALCTX_PENDINGIO_CNT, log);
    // then 1.
    EXPECT_EQ(0, ret);
    EXPECT_EQ(10, log.GetValue());
    // when 2.
    ret = telClient.CollectValue("b", TEL002_ALCT_ALCTX_PENDINGIO_CNT, log);
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

    tp->PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, 10);
    tp->PublishData(TEL001_ALCT_FREE_SEG_CNT, 20);
    tp->PublishData(TEL003_ALCT_GCVICTIM_SEG, 30);
    tp->PublishData(TEL004_ALCT_GCMODE, 1);
    // when
    list<MetricUint32> retList = telClient.CollectList("a");
    // then
    for (auto& p : retList)
    {
        if (p.GetId() == TEL002_ALCT_ALCTX_PENDINGIO_CNT)
        {
            EXPECT_EQ(10, p.GetValue());
        }
        else if (p.GetId() == TEL001_ALCT_FREE_SEG_CNT)
        {
            EXPECT_EQ(20, p.GetValue());
        }
        else if (p.GetId() == TEL003_ALCT_GCVICTIM_SEG)
        {
            EXPECT_EQ(30, p.GetValue());
        }
        else if (p.GetId() == TEL004_ALCT_GCMODE)
        {
            EXPECT_EQ(1, p.GetValue());
        }
    }
    delete tp;
}

} // namespace pos
