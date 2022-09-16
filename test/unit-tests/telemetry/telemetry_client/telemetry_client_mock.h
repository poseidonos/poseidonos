#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
class MockTelemetryClient : public TelemetryClient
{
public:
    using TelemetryClient::TelemetryClient;
    MOCK_METHOD(int, RegisterPublisher, (TelemetryPublisher* tp), (override));
    MOCK_METHOD(int, DeregisterPublisher, (std::string name), (override));
    MOCK_METHOD(bool, StartPublisher, (std::string name), (override));
    MOCK_METHOD(bool, StopPublisher, (std::string name), (override));
    MOCK_METHOD(bool, IsPublisherRunning, (std::string name), (override));
    MOCK_METHOD(bool, StartAllPublisher, (), (override));
    MOCK_METHOD(bool, StopAllPublisher, (), (override));
    MOCK_METHOD(bool, StartUsingDataPool, (std::string name), (override));
    MOCK_METHOD(bool, StopUsingDataPool, (std::string name), (override));
    MOCK_METHOD(bool, StartUsingDataPoolForAllPublisher, (), (override));
    MOCK_METHOD(bool, StopUsingDataPoolForAllPublisher, (), (override));
    MOCK_METHOD(std::string, GetPublicationList, (), (override));
    MOCK_METHOD(bool, IsRunning, (), (override));
};

} // namespace pos
