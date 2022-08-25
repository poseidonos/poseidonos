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

#include "src/pos_replicator/grpc_subscriber.h"

#include <gtest/gtest.h>

#include "src/pos_replicator/dummy_ha/dummy_ha_client.h"
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

class GrpcSubscriberTestFixture : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    NiceMock<MockConfigManager>* configManager;
    GrpcSubscriber* grpcSubscriber;
    DummyHaClient* haClient;
};

void
GrpcSubscriberTestFixture::SetUp(void)
{
    configManager = new NiceMock<MockConfigManager>;
    ON_CALL(*configManager, GetValue("replicator", "ha_subscriber_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("localhost:50053"));

    grpcSubscriber = new GrpcSubscriber(configManager);
    sleep(1);
    haClient = new DummyHaClient(nullptr);
}

void
GrpcSubscriberTestFixture::TearDown(void)
{
    delete configManager;
    delete grpcSubscriber;
    delete haClient;
}

TEST_F(GrpcSubscriberTestFixture, GrpcSubscriber_WriteBlocks)
{
    // Given
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

TEST_F(GrpcSubscriberTestFixture, GrpcSubscriber_WriteHostBlocks)
{
    // Given
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

TEST_F(GrpcSubscriberTestFixture, GrpcSubscriber_ReadBlocks)
{
    // Given
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

TEST_F(GrpcSubscriberTestFixture, GrpcSubscriber_CompleteHostWrite)
{
    // Given
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
} // namespace pos
