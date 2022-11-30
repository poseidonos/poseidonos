/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/pos_replicator/grpc_publisher.h"

#include <gtest/gtest.h>

#include <thread>

#include "test/unit-tests/pos_replicator/mock_grpc/mock_replicator_server.h"
#include "test/unit-tests/pos_replicator/mock_grpc/mock_replicator_client.h"
#include "test/unit-tests/master_context/config_manager_mock.h"

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;

namespace pos
{
ACTION_P(SetArg2ToStringAndReturn0, stringValue)
{
    *static_cast<std::string*>(arg2) = stringValue;
    return 0;
}

class GrpcPublisherTestFixture : public ::testing::Test
{
const int CHUNK_SIZE = 512;
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void* _GenerateDataBlock(int numChunks);

    GrpcPublisher* grpcPublisher;
    MockReplicatorServer* replicatorServer;
    NiceMock<MockConfigManager>* configManager;
    std::string serverAddress;
};

void
GrpcPublisherTestFixture::SetUp(void)
{
    serverAddress = std::string("127.0.0.1:7555");
    configManager = new NiceMock<MockConfigManager>;
    ON_CALL(*configManager, GetValue("replicator", "ha_publisher_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0(serverAddress));

    replicatorServer = new MockReplicatorServer(serverAddress);
    grpcPublisher = new GrpcPublisher(nullptr, configManager);
}

void
GrpcPublisherTestFixture::TearDown(void)
{
    delete configManager;
    delete grpcPublisher;
    delete replicatorServer;
}

void*
GrpcPublisherTestFixture::_GenerateDataBlock(int numChunks)
{
    char * dataBuffer = new char[numChunks * CHUNK_SIZE];
    char* nextChunkBuffer = dataBuffer;
    for(int chunkIndex = 0; chunkIndex < numChunks; chunkIndex++)
    {
        nextChunkBuffer = nextChunkBuffer + (CHUNK_SIZE * chunkIndex);
        for (int offset = 0; offset < CHUNK_SIZE; offset++)
        {
            char c = 'A' + std::rand() % 26;
            nextChunkBuffer[offset] = c;
        }
        nextChunkBuffer[0] = '!' + chunkIndex;
    }
    return (void*)dataBuffer;
}

TEST_F(GrpcPublisherTestFixture, DISABLED_GrpcPublisher_PushHostWrite)
{
    // Given
    uint64_t rba = 10;
    uint64_t size = 10;
    uint64_t lsn;
    string volumeName = "";
    string arrayName = "";
    void* buf;

    // When
    int ret = grpcPublisher->PushHostWrite(rba, size, volumeName, arrayName, buf, lsn);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(GrpcPublisherTestFixture, DISABLED_GrpcPublisher_CompleteUserWrite)
{
    // Given
    uint64_t lsn = 10;
    string volumeName = "";
    string arrayName = "";

    // When
    int ret = grpcPublisher->CompleteUserWrite(lsn, volumeName, arrayName);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(GrpcPublisherTestFixture, DISABLED_GrpcPublisher_CompleteWrite)
{
    // Given
    uint64_t lsn = 10;
    string volumeName = "";
    string arrayName = "";

    // When
    int ret = grpcPublisher->CompleteWrite(lsn, volumeName, arrayName);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(GrpcPublisherTestFixture, DISABLED_GrpcPublisher_CompleteRead)
{
    // Given
    string arrayName = "";
    string volumeName = "";
    uint64_t rba = 0;
    uint64_t numBlocks = 8;
    uint64_t lsn = 10;
    void* buffer = nullptr;

    // When
    int ret = grpcPublisher->CompleteRead(arrayName, volumeName, rba, numBlocks, lsn, buffer);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(GrpcPublisherTestFixture, GrpcPublisher_CompleteReadTestIfPublishDataSuccesfully)
{
    // Given
    string arrayName = const_cast<char*>("Arr0");
    string volumeName = const_cast<char*>("volume1");
    uint64_t rba = 0;
    uint64_t numChunks = 8;
    uint64_t lsn = 10;
    void* buffer = _GenerateDataBlock(numChunks);

    // When
    grpcPublisher->WaitClientConnected();
    int ret = grpcPublisher->CompleteRead(arrayName, volumeName, rba, numChunks, lsn, buffer);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}
} // namespace pos
