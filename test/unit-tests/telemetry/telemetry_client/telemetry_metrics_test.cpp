#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_id.h"
#include "src/telemetry/telemetry_client/telemetry_metrics.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(Metric, GetId_)
{
    // given
    Metric* basetype = new Metric();
    time_t cur = std::time(nullptr);
    std::string timestring = "time";
    basetype->SetCommonMetric(TEL003_ALCT_GCVICTIM_SEG, cur, timestring);
    // when
    std::string ret = basetype->GetId();
    // then
    EXPECT_EQ(TEL003_ALCT_GCVICTIM_SEG, ret);
    delete basetype;
}

TEST(Metric, GetTime_)
{
    // given
    Metric* basetype = new Metric();
    time_t cur = std::time(nullptr);
    std::string timestring = "time";
    basetype->SetCommonMetric(TEL003_ALCT_GCVICTIM_SEG, cur, timestring);
    // when
    time_t ret = basetype->GetTime();
    // then
    EXPECT_EQ(cur, ret);
    delete basetype;
}

TEST(Metric, GetTimeString_)
{
    // given
    Metric* basetype = new Metric();
    time_t cur = std::time(nullptr);
    std::string timestring = "time";
    basetype->SetCommonMetric(TEL003_ALCT_GCVICTIM_SEG, cur, timestring);
    // when
    std::string ret = basetype->GetTimeString();
    // then
    EXPECT_EQ("time", ret);
    delete basetype;
}

} // namespace pos

namespace pos
{
TEST(MetricUint32, GetValue_)
{
    // given
    MetricUint32* metric = new MetricUint32();
    time_t cur = std::time(nullptr);
    std::string timestring = "time";
    metric->SetMetric(TEL003_ALCT_GCVICTIM_SEG, cur, 50, timestring);
    // when
    uint32_t ret = metric->GetValue();
    // then
    EXPECT_EQ(50, ret);
    delete metric;
}

} // namespace pos

namespace pos
{
TEST(MetricString, GetValue_)
{
    // given
    MetricString* metric = new MetricString();
    time_t cur = std::time(nullptr);
    std::string val = "helloWorld";
    std::string timestring = "time";
    metric->SetMetric("airString", cur, val, timestring);
    // when
    std::string ret = metric->GetValue();
    // then
    EXPECT_EQ(val, ret);
    delete metric;
}

} // namespace pos
