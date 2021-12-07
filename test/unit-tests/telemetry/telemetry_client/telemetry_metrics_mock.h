#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/pos_metric.h"

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

class MockPOSMetric : public POSMetric
{
public:
    using POSMetric::POSMetric;
};

class MockMetricString : public MetricString
{
public:
    using MetricString::MetricString;
};

} // namespace pos
