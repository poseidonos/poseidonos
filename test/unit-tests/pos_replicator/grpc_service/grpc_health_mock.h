#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/grpc_service/grpc_health.h"

namespace pos
{
class MockGrpcHealth : public GrpcHealth
{
public:
    using GrpcHealth::GrpcHealth;
    MOCK_METHOD(::grpc::Status, Check, (::grpc::ServerContext * context, const pos_rpc::HealthCheckRequest* request, pos_rpc::HealthCheckResponse* response), (override));
};

} // namespace pos
