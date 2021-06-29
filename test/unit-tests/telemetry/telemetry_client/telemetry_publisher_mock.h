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
    MOCK_METHOD(int, PublishData, (std::string id, uint32_t value), (override));
    MOCK_METHOD(int, CollectData, (std::string id, TelemetryGeneralMetric& outLog), (override));
    MOCK_METHOD(list<TelemetryGeneralMetric>, CollectAll, (), (override));
};

} // namespace pos
