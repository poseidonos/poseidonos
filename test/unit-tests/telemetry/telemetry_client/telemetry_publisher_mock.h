#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class MockTelemetryPublisher : public TelemetryPublisher
{
public:
    using TelemetryPublisher::TelemetryPublisher;
    MOCK_METHOD(void, StartPublishing, (), (override));
    MOCK_METHOD(void, StopPublishing, (), (override));
    MOCK_METHOD(bool, IsRunning, (), (override));
    MOCK_METHOD(void, StartUsingDataPool, (), (override));
    MOCK_METHOD(void, StopUsingDataPool, (), (override));
    MOCK_METHOD(void, SetMaxEntryLimit, (int limit), (override));
    MOCK_METHOD(int, PublishData, (std::string id_, POSMetricValue value_, POSMetricTypes type_), (override));
    MOCK_METHOD(int, PublishMetric, (POSMetric metric), (override));
    MOCK_METHOD(int, PublishMetricList, (POSMetricVector* metricList), (override));
    MOCK_METHOD(void, SetGlobalPublisher, (IGlobalPublisher * gp), (override));
};

} // namespace pos
