#include "src/telemetry/telemetry_client/telemetry_client.h"

#include <gtest/gtest.h>

#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
TEST(TelemetryClient, RegisterClient_TestRegisterDeregister)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher("aaa");
    TelemetryClient telClient;
    // when 1.
    int ret = telClient.RegisterPublisher(tp);
    // then 2.
    EXPECT_EQ(0, ret);
    // given 3.
    telClient.DeregisterPublisher(tp->GetName());
    delete tp;
}

TEST(TelemetryClient, StartPublisher_TestStartAndStop)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher("a");
    TelemetryClient telClient;
    telClient.RegisterPublisher(tp);
    // when 1.
    telClient.StartPublisher(tp->GetName());
    // then 1.
    bool ret = telClient.IsPublisherRunning(tp->GetName());
    EXPECT_EQ(true, ret);
    // when 2.
    telClient.StopPublisher(tp->GetName());
    // then 2.
    ret = telClient.IsPublisherRunning(tp->GetName());
    EXPECT_EQ(false, ret);
    delete tp;
}

TEST(TelemetryClient, StartTelemetryPublisherAll_TestStartAndStopAll)
{
    // given
    TelemetryPublisher* tp = new TelemetryPublisher("a");
    TelemetryPublisher* tp2 = new TelemetryPublisher("b");
    TelemetryClient telClient;
    telClient.RegisterPublisher(tp);
    telClient.RegisterPublisher(tp2);
    // when 1.
    telClient.StartAllPublisher();
    // then 1.
    bool ret = telClient.IsPublisherRunning(tp->GetName());
    EXPECT_EQ(true, ret);
    ret = telClient.IsPublisherRunning(tp2->GetName());
    EXPECT_EQ(true, ret);
    // when 2.
    telClient.StopAllPublisher();
    // then 2.
    ret = telClient.IsPublisherRunning(tp->GetName());
    EXPECT_EQ(false, ret);
    ret = telClient.IsPublisherRunning(tp2->GetName());
    EXPECT_EQ(false, ret);
    delete tp;
    delete tp2;
}

} // namespace pos
