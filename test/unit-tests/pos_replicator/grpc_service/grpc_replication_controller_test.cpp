#include "src/pos_replicator/grpc_service/grpc_replication_controller.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/unit-tests/pos_replicator/posreplicator_manager_mock.h"

using ::testing::NiceMock;

namespace pos
{
class GrpcReplicationControllerTestFixture : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    GrpcReplicationController* grpcReplicationController;
    NiceMock<MockPosReplicatorManager> replicatorManager;
};

void
GrpcReplicationControllerTestFixture::SetUp(void)
{
    grpcReplicationController = new GrpcReplicationController(&replicatorManager);
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
