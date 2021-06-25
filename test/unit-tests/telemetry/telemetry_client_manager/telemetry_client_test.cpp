#include "src/telemetry/telemetry_client_manager/telemetry_client.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(TelemetryClient, PublishData_)
{
    // given
    TelemetryClient tc;
    // given 1.
    tc.StopLogging();
    // when 1.
    int ret = tc.PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 100);
    // then 1.
    EXPECT_EQ(-1, ret);
    // given 2.
    tc.StartLogging();
    // when 2.
    ret = tc.PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 200);
    // then 2.
    EXPECT_EQ(0, ret);
    // given 3.
    tc.PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 10);
    tc.PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 9);
    TelemetryLogEntry log;
    // when 3.
    ret = tc.CollectData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, log);
    // then 3.
    EXPECT_EQ(9, log.GetValue());
    // when 4.
    ret = tc.CollectData("ccc", log);
    // then 4.
    EXPECT_EQ(-1, ret);
}

} // namespace pos
