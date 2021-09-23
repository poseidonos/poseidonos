#include "src/telemetry/telemetry_client/telemetry_data_pool.h"
#include "src/telemetry/telemetry_client/telemetry_metrics.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_data_pool_mock.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(TelemetryDataPool, DataPool_TestAll)
{
    // given
    TelemetryDataPool pool;
    MetricUint32 metric;
    MetricUint32 metric2;
    metric.SetMetric("aaa", std::time(nullptr), 5, "time");
    metric2.SetMetric("bbb", std::time(nullptr), 6, "time");
    pool.SetLog(metric);
    pool.SetLog(metric2);
    MetricUint32 log;
    pool.GetLog("aaa", log);
    time_t t = log.GetTime();
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
