#include "src/pos_replicator/grpc_service/grpc_pos_management.h"

#include <gtest/gtest.h>

namespace pos
{
class GrpcPosManagementTestFixture : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    GrpcPosManagement* grpcPosManagement;
};


void
GrpcPosManagementTestFixture::SetUp(void)
{
    grpcPosManagement = new GrpcPosManagement();
}

void
GrpcPosManagementTestFixture::TearDown(void)
{
    delete grpcPosManagement;
}

TEST(GrpcPosManagementTestFixture, GrpcPosManagement_)
{
}

TEST(GrpcPosManagementTestFixture, UpdateVoluemMeta_)
{
}

TEST(GrpcPosManagementTestFixture, GetVolumeList_)
{
}

} // namespace pos
