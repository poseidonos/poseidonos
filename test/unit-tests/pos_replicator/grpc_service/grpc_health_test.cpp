#include "src/pos_replicator/grpc_service/grpc_health.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
using ::testing::_;

namespace pos
{
TEST(GrpcHealth, GrpcHealth_)
{
    GrpcHealth grpcHealth;
}

TEST(GrpcHealth, Check_)
{
    // Given
    ::grpc::ServerContext* context;
    pos_rpc::HealthCheckRequest* request;
    pos_rpc::HealthCheckResponse response;
    // When
    GrpcHealth grpcHealth;
    grpc::Status ret = grpcHealth.Check(context, request, &response);
    // Then
    EXPECT_EQ(ret.ok(), true);
}

} // namespace pos
