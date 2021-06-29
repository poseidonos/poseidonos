#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
class MockTelemetryPublisher : public TelemetryClient
{
public:
    using TelemetryClient::TelemetryClient;
    MOCK_METHOD(int, RegisterPublisher, (std::string name, TelemetryPublisher* publisher), (override));
    MOCK_METHOD(int, DeregisterPublisher, (std::string name), (override));
    MOCK_METHOD(void, StartPublisher, (std::string name), (override));
    MOCK_METHOD(void, StopPublisher, (std::string name), (override));
    MOCK_METHOD(bool, IsPublisherRunning, (std::string name), (override));
    MOCK_METHOD(void, StartAllPublisher, (), (override));
    MOCK_METHOD(void, StopAllPublisher, (), (override));
    MOCK_METHOD(int, CollectValue, (std::string name, std::string id, TelemetryGeneralMetric& outLog), (override));
    MOCK_METHOD(list<TelemetryGeneralMetric>, CollectList, (), (override));
};

} // namespace pos
