#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client_manager/telemetry_client.h"

namespace pos
{
class MockTelemetryClient : public TelemetryClient
{
public:
    using TelemetryClient::TelemetryClient;
    MOCK_METHOD(void, StartLogging, (), (override));
    MOCK_METHOD(void, StopLogging, (), (override));
    MOCK_METHOD(bool, IsRunning, (), (override));
    MOCK_METHOD(int, PublishData, (std::string id, uint32_t value), (override));
    MOCK_METHOD(int, CollectData, (std::string id, TelemetryLogEntry& outLog), (override));
};

} // namespace pos
