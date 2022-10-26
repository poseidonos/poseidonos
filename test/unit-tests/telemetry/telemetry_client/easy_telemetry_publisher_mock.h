#include <gmock/gmock.h>

#include <string>
#include <utility>
#include <vector>

#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"

namespace pos
{
class MockEasyTelemetryPublisher : public EasyTelemetryPublisher
{
public:
    using EasyTelemetryPublisher::EasyTelemetryPublisher;

    MOCK_METHOD(void, IncreaseCounter, (const std::string&));
    MOCK_METHOD(void, IncreaseCounter, (const std::string&, const uint64_t));
    MOCK_METHOD(void, IncreaseCounter, (const std::string&, const uint64_t, const VectorLabels&));
    MOCK_METHOD(void, UpdateGauge, (const std::string&, const uint64_t));
    MOCK_METHOD(void, UpdateGauge, (const std::string&, const uint64_t, const VectorLabels&));
    MOCK_METHOD(void, PublishMetricsWithLabels, ());
    MOCK_METHOD(void, UpdateMetrics, ());
};

} // namespace pos
