#include "src/pos_replicator/grpc_service/grpc_replication_controller.h"

#include <gtest/gtest.h>

namespace pos
{
class GrpcReplicationControllerTestFixture : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    GrpcReplicationController* grpcReplicationController;
};

void
GrpcReplicationControllerTestFixture::SetUp(void)
{
    grpcReplicationController = new GrpcReplicationController();
}

void
GrpcReplicationControllerTestFixture::TearDown(void)
{
    delete grpcReplicationController;
}

TEST(GrpcReplicationControllerTestFixture, GrpcReplicationController_)
{
}

TEST(GrpcReplicationControllerTestFixture, StartVolumeSync_)
{
}

} // namespace pos
