#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client_manager/telemetry_client_manager.h"

namespace pos
{
class MockTelemetryClientManager : public TelemetryClientManager
{
public:
    using TelemetryClientManager::TelemetryClientManager;
    MOCK_METHOD(int, RegisterClient, (std::string name, TelemetryClient* client), (override));
    MOCK_METHOD(int, DeregisterClient, (std::string name), (override));
    MOCK_METHOD(void, StartTelemetryClient, (std::string name), (override));
    MOCK_METHOD(void, StopTelemetryClient, (std::string name), (override));
    MOCK_METHOD(bool, IsTelemetryClientRunning, (std::string name), (override));
    MOCK_METHOD(void, StartTelemetryClientAll, (), (override));
    MOCK_METHOD(void, StopTelemetryClientAll, (), (override));
    MOCK_METHOD(int, CollectData, (std::string name, std::string id, TelemetryLogEntry& outLog), (override));
};

} // namespace pos
