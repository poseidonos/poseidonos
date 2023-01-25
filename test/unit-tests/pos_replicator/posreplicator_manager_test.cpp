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

#include "src/pos_replicator/posreplicator_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "mock_grpc/mock_replicator_client.h"
#include "mock_grpc/mock_replicator_server.h"
#include "spdk/pos.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/io/frontend_io/aio_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/pos_replicator/grpc_publisher_mock.h"
#include "test/unit-tests/pos_replicator/grpc_subscriber_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using namespace ::testing;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
ACTION_P(SetArg2ToStringAndReturn0, stringValue)
{
    *static_cast<std::string*>(arg2) = stringValue;
    return 0;
}

class PosReplicatorManagerTestFixture : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    NiceMock<MockAIO>* aio;
    NiceMock<MockConfigManager>* configManager;
    PosReplicatorManager* posReplicatorManager;
    NiceMock<MockGrpcPublisher>* grpcPublisher;
    NiceMock<MockGrpcSubscriber>* grpcSubscriber;
    NiceMock<MockTelemetryPublisher> tp;
};

void
PosReplicatorManagerTestFixture::SetUp(void)
{
    // new PosReplicator : POS side
    aio = new NiceMock<MockAIO>;
    configManager = new NiceMock<MockConfigManager>;
    ON_CALL(*configManager, GetValue("replicator", "ha_publisher_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50003"));
    ON_CALL(*configManager, GetValue("replicator", "ha_subscriber_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50053"));

    posReplicatorManager = new PosReplicatorManager(aio, &tp);
    grpcPublisher = new NiceMock<MockGrpcPublisher>(nullptr, configManager);
    grpcSubscriber = new NiceMock<MockGrpcSubscriber>(posReplicatorManager, configManager);
    posReplicatorManager->Init(grpcPublisher, grpcSubscriber, configManager);
}

void
PosReplicatorManagerTestFixture::TearDown(void)
{
    delete aio;
    delete configManager;
    delete grpcPublisher;
    delete grpcSubscriber;
    posReplicatorManager->Dispose();
    delete posReplicatorManager;
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_CompelteUserIO_)
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    int ret = posReplicatorManager->CompleteUserIO(lsn, arrayId, volumeId);

    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_HAIOSubmission_testIfIoTypeIsReadWithSuccesfully)
{
    // Given: Read IO Requested
    // new Server : HA side
    // haServer = new MockReplicatorServer();
    // string serverAddress("0.0.0.0:0");
    // new std::thread(&MockReplicatorServer::RunServer, haServer, serverAddress);

    // Read IO
    IO_TYPE ioType = IO_TYPE::READ;
    int arrayId = 1;
    int volumeId = 0;
    uint64_t rba = 10;
    uint64_t numChunks = 8; // 4KB
    uint64_t lsn = 0;
    std::shared_ptr<char*> dataList = nullptr;

    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, numChunks, arrayId));
    EXPECT_CALL(*aio, CreatePosReplicatorVolumeIo).WillOnce(Return(volumeIo));
    EXPECT_CALL(*aio, SubmitAsyncIO(volumeIo));

    // Then
    int ret = posReplicatorManager->HAIOSubmission(ioType, arrayId, volumeId, rba, numChunks, dataList, lsn);
    EXPECT_EQ(EID(SUCCESS), ret);
    // delete haServer;
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_HAIOCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    uint64_t rba = 0;
    uint64_t numChunks = 8;

    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    posReplicatorManager->HAIOCompletion(lsn, volumeIo, rba, numChunks);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_HAWriteCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    uint64_t rba = 0;
    uint64_t numChunks = 8;

    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    posReplicatorManager->HAWriteCompletion(lsn, volumeIo, rba, numChunks);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_HAReadCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    uint64_t rba = 0;
    uint64_t numChunks = 8;

    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    posReplicatorManager->HAReadCompletion(lsn, volumeIo, rba, numChunks);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_ConvertIdToName_)
{
    // Given
    // When
    int arrayId = 0;
    int volumeId = 0;
    std::string arrayName;
    std::string volumeName;

    int ret = posReplicatorManager->ConvertIdToName(arrayId, volumeId, arrayName, volumeName);

    // Then
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_ConvertNameToId_)
{
    // Given
    // When
    int arrayId = HA_INVALID_ARRAY_IDX;
    int volumeId = HA_INVALID_VOLUME_IDX;
    std::string arrayName;
    std::string volumeName;
    std::pair<std::string, int> arraySet(arrayName, arrayId);
    std::pair<std::string, int> volumeSet(volumeName, volumeId);

    int ret = posReplicatorManager->ConvertNameToIdx(arraySet, volumeSet);

    // Then
    EXPECT_NE(EID(SUCCESS), ret);
}
} // namespace pos
