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

#include "src/pos_replicator/dummy_ha/dummy_ha_server.h"
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
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    DummyHaServer* haServer;
    GrpcPublisher* posClient;
    NiceMock<MockConfigManager>* configManager;
};

void
GrpcPublisherTestFixture::SetUp(void)
{
    configManager = new NiceMock<MockConfigManager>;
    ON_CALL(*configManager, GetValue("replicator", "ha_publisher_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50003"));
    
    // new Server : HA side
    haServer = new DummyHaServer();
    string serverAddress(GRPC_HA_PUB_SERVER_SOCKET_ADDRESS);
    new std::thread(&DummyHaServer::RunServer, haServer, serverAddress);
    sleep(1);

    posClient = new GrpcPublisher(nullptr, configManager);
}

void
GrpcPublisherTestFixture::TearDown(void)
{
    delete configManager;
    delete haServer;
    sleep(1);

    delete posClient;
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
    int ret = posClient->PushHostWrite(rba, size, volumeName, arrayName, buf, lsn);

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
    int ret = posClient->CompleteUserWrite(lsn, volumeName, arrayName);

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
    int ret = posClient->CompleteWrite(lsn, volumeName, arrayName);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(GrpcPublisherTestFixture, DISABLED_GrpcPublisher_CompleteRead)
{
    // Given
    uint64_t lsn = 10;
    uint64_t size = 10;
    string volumeName = "";
    string arrayName = "";
    void* buf = nullptr;

    // When
    int ret = posClient->CompleteRead(lsn, size, volumeName, arrayName, buf);

    // Then: Do Nothing
    EXPECT_EQ(EID(SUCCESS), ret);
}

} // namespace pos
