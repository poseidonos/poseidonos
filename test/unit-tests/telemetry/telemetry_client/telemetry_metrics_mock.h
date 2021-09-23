#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_metrics.h"

namespace pos
{
class MockMetric : public Metric
{
public:
    using Metric::Metric;
    MOCK_METHOD(std::string, GetId, (), (override));
    MOCK_METHOD(time_t, GetTime, (), (override));
    MOCK_METHOD(std::string, GetTimeString, (), (override));
    MOCK_METHOD(void, SetCommonMetric, (std::string id_, time_t t_, std::string st_), (override));
};

class MockMetricUint32 : public MetricUint32
{
public:
    using MetricUint32::MetricUint32;
};

class MockMetricString : public MetricString
{
public:
    using MetricString::MetricString;
};

} // namespace pos
