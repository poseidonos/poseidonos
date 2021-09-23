#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/telemetry_data_pool.h"

namespace pos
{
class MockTelemetryDataPool : public TelemetryDataPool
{
public:
    using TelemetryDataPool::TelemetryDataPool;
};

} // namespace pos
