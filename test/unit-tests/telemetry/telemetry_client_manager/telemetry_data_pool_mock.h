#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client_manager/telemetry_data_pool.h"

namespace pos
{
class MockTelemetryLogEntry : public TelemetryLogEntry
{
public:
    using TelemetryLogEntry::TelemetryLogEntry;
};

class MockTelemetryDataPool : public TelemetryDataPool
{
public:
    using TelemetryDataPool::TelemetryDataPool;
};

} // namespace pos
