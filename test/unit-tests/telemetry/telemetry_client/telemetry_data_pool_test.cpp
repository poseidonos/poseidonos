#include "src/telemetry/telemetry_client/telemetry_data_pool.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(TelemetryDataPool, DataPool_TestAll)
{
    // given
    TelemetryDataPool pool;
    pool.SetLog("aaa", 5);
    pool.SetLog("bbb", 6);
    TelemetryGeneralMetric log;
    pool.GetLog("aaa", log);
    tm t = log.GetTime();
    std::string st = log.GetTimeString();
    // when 1.
    int ret = pool.GetNumEntries();
    // then 1.
    EXPECT_EQ(2, ret);
    // when 2.
    ret = pool.GetLog("ccc", log);
    // then 2.
    EXPECT_EQ(-1, ret);
}

} // namespace pos
