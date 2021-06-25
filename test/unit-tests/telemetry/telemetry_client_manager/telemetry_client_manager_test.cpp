#include "src/telemetry/telemetry_client_manager/telemetry_client_manager.h"

#include <gtest/gtest.h>

#include "src/telemetry/telemetry_client_manager/telemetry_client.h"
#include "test/unit-tests/telemetry/telemetry_client_manager/telemetry_client_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(TelemetryClientManager, RegisterClient_TestRegisterDeregister)
{
    // given
    NiceMock<TelemetryClient>* tc = new NiceMock<TelemetryClient>();
    TelemetryClientManager telClient;
    // when 1.
    int ret = telClient.RegisterClient("a", tc);
    // then 2.
    EXPECT_EQ(0, ret);
    // when 2.
    ret = telClient.RegisterClient("a", tc);
    // then 2.
    EXPECT_EQ(-1, ret);
    // given 3.
    telClient.DeregisterClient("a");
    telClient.DeregisterClient("a");
    // when 3.
    ret = telClient.RegisterClient("a", tc);
    // then 3.
    EXPECT_EQ(0, ret);
    delete tc;
}

TEST(TelemetryClientManager, StartTelemetryClient_TestStartAndStop)
{
    // given
    TelemetryClient* tc = new TelemetryClient();
    TelemetryClientManager telClient;
    telClient.RegisterClient("a", tc);
    // when 1.
    telClient.StartTelemetryClient("a");
    // then 1.
    bool ret = telClient.IsTelemetryClientRunning("a");
    EXPECT_EQ(true, ret);
    // when 2.
    telClient.StopTelemetryClient("a");
    // then 2.
    ret = telClient.IsTelemetryClientRunning("a");
    EXPECT_EQ(false, ret);
    delete tc;
}

TEST(TelemetryClientManager, StartTelemetryClientAll_TestStartAndStopAll)
{
    // given
    TelemetryClient* tc = new TelemetryClient();
    TelemetryClient* tc2 = new TelemetryClient();
    TelemetryClientManager telClient;
    telClient.RegisterClient("a", tc);
    telClient.RegisterClient("b", tc2);
    // when 1.
    telClient.StartTelemetryClientAll();
    // then 1.
    bool ret = telClient.IsTelemetryClientRunning("a");
    EXPECT_EQ(true, ret);
    ret = telClient.IsTelemetryClientRunning("b");
    EXPECT_EQ(true, ret);
    // when 2.
    telClient.StopTelemetryClientAll();
    // then 2.
    ret = telClient.IsTelemetryClientRunning("a");
    EXPECT_EQ(false, ret);
    ret = telClient.IsTelemetryClientRunning("b");
    EXPECT_EQ(false, ret);
    delete tc;
    delete tc2;
}

TEST(TelemetryClientManager, CollectData_TestData)
{
    // given
    TelemetryClient* tc = new TelemetryClient();
    TelemetryClientManager telClient;
    telClient.RegisterClient("a", tc);
    telClient.StartTelemetryClient("a");
    tc->PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 10);
    TelemetryLogEntry log;
    // when 1.
    int ret = telClient.CollectData("a", TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, log);
    // then 1.
    EXPECT_EQ(0, ret);
    EXPECT_EQ(10, log.GetValue());
    // when 2.
    ret = telClient.CollectData("b", TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, log);
    // then 2.
    EXPECT_EQ(-1, ret);
    // when 3.
    ret = telClient.CollectData("a", "noname", log);
    // then 3.
    EXPECT_EQ(-1, ret);
    delete tc;
}

} // namespace pos
