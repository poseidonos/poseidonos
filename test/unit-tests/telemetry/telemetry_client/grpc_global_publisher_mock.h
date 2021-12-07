#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/telemetry/telemetry_client/grpc_global_publisher.h"

namespace pos
{
class MockGrpcGlobalPublisher : public GrpcGlobalPublisher
{
public:
    using GrpcGlobalPublisher::GrpcGlobalPublisher;
    MOCK_METHOD(int, PublishToServer, (POSMetric & metric), (override));
    MOCK_METHOD(int, PublishToServer, (MetricString & metric), (override));
};

} // namespace pos
