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
    MOCK_METHOD(int, GetNumEntries, (), (override));
    MOCK_METHOD(int, PublishData, (std::string id, uint32_t value), (override));
    //MOCK_METHOD(int, PublishData, (std::string id, std::string value), (override));
    MOCK_METHOD(int, CollectData, (std::string id, MetricUint32& outLog), (override));
    MOCK_METHOD(list<MetricUint32>, CollectAll, (), (override));
    MOCK_METHOD(void, SetGlobalPublisher, (IGlobalPublisher * gp), (override));
};

} // namespace pos
