#include "src/pos_replicator/grpc_subscriber.h"
#include "src/pos_replicator/dummy_ha/dummy_ha_client.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(GrpcSubscriber, GrpcSubscriber_WriteBlocks)
{
    // Given
    // server_address("0.0.0.0:50051");
    GrpcSubscriber *posServer = new GrpcSubscriber();
    usleep(1000);
    // client
    DummyHaClient *haClient = new DummyHaClient(nullptr);

    // When
    ::grpc::Status status = haClient->WriteBlocks();

    // Then: Do Nothing
    bool ret = false;

    if (status.ok() != true)
    {
        ret = true;
    }

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}

TEST(GrpcSubscriber, GrpcSubscriber_WriteHostBlocks)
{
    // Given
    // server_address("0.0.0.0:50051");
    GrpcSubscriber *posServer = new GrpcSubscriber();
    usleep(1000);
    // client
    DummyHaClient *haClient = new DummyHaClient(nullptr);

    // When
    ::grpc::Status status = haClient->WriteHostBlocks();

    bool ret = false;

    if (status.ok() != true)
    {
        ret = true;
    }

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}

TEST(GrpcSubscriber, GrpcSubscriber_ReadBlocks)
{
    // Given
    // server_address("0.0.0.0:50051");
    GrpcSubscriber *posServer = new GrpcSubscriber();
    usleep(1000);
    // client
    DummyHaClient *haClient = new DummyHaClient(nullptr);

    // When
    ::grpc::Status status = haClient->ReadBlocks();

    // Then: Do Nothing
    bool ret = false;

    if (status.ok() != true)
    {
        ret = true;
    }

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}

TEST(GrpcSubscriber, GrpcSubscriber_CompleteHostWrite)
{
    // Given
    // server_address("0.0.0.0:50051");
    GrpcSubscriber *posServer = new GrpcSubscriber();
    usleep(1000);
    // client
    DummyHaClient *haClient = new DummyHaClient(nullptr);

    // When
    ::grpc::Status status = haClient->CompleteHostWrite();

    // Then: Do Nothing
    bool ret = false;

    if (status.ok() != true)
    {
        ret = true;
    }

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}
}