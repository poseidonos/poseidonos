

#include <gtest/gtest.h>

#include "src/pos_replicator/grpc_publisher.h"
#include "src/pos_replicator/dummy_ha/dummy_ha_server.h"


namespace pos
{

TEST(GrpcPublisher, GrpcPublisher_PushHostWrite)
{
    // Given
    // server_address("0.0.0.0:50051");
    DummyHaServer *haServer = new DummyHaServer();
    usleep(1000);
    // client
    GrpcPublisher *posClient = new GrpcPublisher(nullptr);

    uint64_t rba = 10;
    uint64_t size = 10;    
    string volumeName = "";
    string arrayName = "";
    void* buf;

    // When
    uint64_t ret = posClient->PushHostWrite(rba, size, volumeName, arrayName, buf);

    // Then: Do Nothing
    EXPECT_EQ(10, ret);
}

TEST(GrpcPublisher, GrpcPublisher_CompleteUserWrite)
{
    // Given
    // server_address("0.0.0.0:50051");
    DummyHaServer *haServer = new DummyHaServer();
    usleep(1000);
    // client
    GrpcPublisher *posClient = new GrpcPublisher(nullptr);
    uint64_t lsn = 10;
    string volumeName = "";
    string arrayName = "";

    // When
    bool ret = posClient->CompleteUserWrite(lsn, volumeName, arrayName);

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}

TEST(GrpcPublisher, GrpcPublisher_CompleteWrite)
{
    // Given
    // server_address("0.0.0.0:50051");
    DummyHaServer *haServer = new DummyHaServer();
    usleep(1000);
    // client
    GrpcPublisher *posClient = new GrpcPublisher(nullptr);
    uint64_t lsn = 10;
    string volumeName = "";
    string arrayName = "";

    // When
    bool ret = posClient->CompleteWrite(lsn, volumeName, arrayName);

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}

TEST(GrpcPublisher, GrpcPublisher_CompleteRead)
{
    // Given
    // server_address("0.0.0.0:50051");
    DummyHaServer *haServer = new DummyHaServer();
    usleep(1000);
    // client
    GrpcPublisher *posClient = new GrpcPublisher(nullptr);
    uint64_t lsn = 10;
    uint64_t size = 10;
    string volumeName = "";
    string arrayName = "";
    void* buf = nullptr;

    // When
    bool ret = posClient->CompleteRead(lsn, size, volumeName, arrayName, buf);

    // Then: Do Nothing
    EXPECT_EQ(true, ret);
}

}