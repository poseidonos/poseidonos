#include "src/pos_replicator/grpc_service/grpc_pos_io.h"

#include <gtest/gtest.h>

namespace pos
{
class GrpcPosIoTestFixture : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    GrpcPosIo* grpcPosIo;
};


void
GrpcPosIoTestFixture::SetUp(void)
{
    grpcPosIo = new GrpcPosIo();
}

void
GrpcPosIoTestFixture::TearDown(void)
{
    delete grpcPosIo;
}


TEST(GrpcPosIoTestFixture, GrpcPosIo_)
{
}

TEST(GrpcPosIoTestFixture, WriteBlocks_)
{
}

TEST(GrpcPosIoTestFixture, WriteBlocksSync_)
{
}

TEST(GrpcPosIoTestFixture, WriteHostBlocks_)
{
}

TEST(GrpcPosIoTestFixture, WriteHostBlocksSync_)
{
}

TEST(GrpcPosIoTestFixture, ReadBlocks_)
{
}

TEST(GrpcPosIoTestFixture, ReadBlocksSync_)
{
}

TEST(GrpcPosIoTestFixture, CompleteHostWrite_)
{
}

TEST(GrpcPosIoTestFixture, _CheckArgumentValidityAndUpdateIndex_)
{
}

} // namespace pos
