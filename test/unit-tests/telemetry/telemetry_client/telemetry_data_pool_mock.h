#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_data_pool.h"

namespace pos
{
class MockMetricUint32 : public MetricUint32
{
public:
    using MetricUint32::MetricUint32;
};

class MockTelemetryDataPool : public TelemetryDataPool
{
public:
    using TelemetryDataPool::TelemetryDataPool;
};

} // namespace pos
