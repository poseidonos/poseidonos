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

#include "spdk/pos.h"
#include "src/include/pos_event_id.h"
#include "mock_grpc/mock_replicator_client.h"
#include "mock_grpc/mock_replicator_server.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/pos_replicator/grpc_publisher_mock.h"
#include "test/unit-tests/pos_replicator/grpc_subscriber_mock.h"
#include "test/unit-tests/io/frontend_io/aio_mock.h"

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

    NiceMock<MockAIO> *aio;
    MockReplicatorServer* haServer;
    MockReplicatorClient* haClient;
    NiceMock<MockConfigManager>* configManager;
    PosReplicatorManager* posReplicatorManager;
    NiceMock<MockGrpcPublisher>* grpcPublisher;
    NiceMock<MockGrpcSubscriber>* grpcSubscriber;
};

void
PosReplicatorManagerTestFixture::SetUp(void)
{
    // new PosReplicator : POS side
    aio = new NiceMock<MockAIO>;
    configManager = new NiceMock<MockConfigManager>;
    ON_CALL(*configManager, GetValue("replicator", "ha_publisher_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50003"));
    ON_CALL(*configManager, GetValue("replicator", "ha_subscriber_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50053"));

    posReplicatorManager = new PosReplicatorManager(aio);
    grpcPublisher = new NiceMock<MockGrpcPublisher>(nullptr, configManager);
    grpcSubscriber = new NiceMock<MockGrpcSubscriber>(configManager);
    posReplicatorManager->Init(grpcPublisher, grpcSubscriber);

    // new Client : HA side
    haClient = new MockReplicatorClient(nullptr);
}

void
PosReplicatorManagerTestFixture::TearDown(void)
{
    delete aio;
    delete configManager;
    delete haServer;
    delete haClient;
    posReplicatorManager->Dispose();
    delete posReplicatorManager;
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_NotifyNewUserIORequest_)
{
    // Given

    pos_io io;
    io.ioType = IO_TYPE::WRITE;
    io.array_id = 0;
    io.volume_id = 0;
    io.arrayName = "ARR0";

    // Then
    int ret = posReplicatorManager->NotifyNewUserIORequest(io);

    EXPECT_NE(EID(SUCCESS), ret);
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

TEST_F(PosReplicatorManagerTestFixture, DISABLED_UserVolumeWriteSubmission_) // To do
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    int ret = posReplicatorManager->UserVolumeWriteSubmission(lsn, arrayId, volumeId);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, HAIOSubmission_testIfIoTypeIsReadWithSuccesfully)
{
    // Given: Read IO Requested
    // new Server : HA side
    haServer = new MockReplicatorServer();
    string serverAddress("0.0.0.0:0");
    new std::thread(&MockReplicatorServer::RunServer, haServer, serverAddress);

    // Read IO
    IO_TYPE ioType = IO_TYPE::READ;
    int arrayId = 1;
    int volumeId = 0;
    uint64_t rba = 10;
    uint64_t numChunks = 8; // 4KB
    std::shared_ptr<char*> dataList = nullptr;

    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, numChunks, arrayId));
    EXPECT_CALL(*aio, CreatePosReplicatorVolumeIo).WillOnce(Return(volumeIo));
    EXPECT_CALL(*aio, SubmitAsyncIO(volumeIo));

    // Then
    int ret = posReplicatorManager->HAIOSubmission(ioType, arrayId, volumeId, rba, numChunks, dataList);
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, HAIOCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    posReplicatorManager->HAIOCompletion(lsn, volumeIo);
}

TEST_F(PosReplicatorManagerTestFixture, HAWriteCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    posReplicatorManager->HAWriteCompletion(lsn, volumeIo);
}

TEST_F(PosReplicatorManagerTestFixture, HAReadCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    posReplicatorManager->HAReadCompletion(lsn, volumeIo);
}

TEST_F(PosReplicatorManagerTestFixture, ConvertArrayIdtoArrayName_)
{
    // Given

    // Then
    int arrayId;
    string arrayName;
    int ret = posReplicatorManager->ConvertArrayIdtoArrayName(arrayId, arrayName);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, ConvertVolumeIdtoVolumeName_)
{
    // Given
    int volumeId;
    int arrayId;
    string volumeName;

    // Then
    int ret = posReplicatorManager->ConvertVolumeIdtoVolumeName(volumeId, arrayId, volumeName);
}

TEST_F(PosReplicatorManagerTestFixture, ConvertArrayNametoArrayId_)
{
    // Then
    int volumeId;
    int arrayId;
    std::string arrayName;
    int ret = posReplicatorManager->ConvertArrayNametoArrayId(arrayName);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, ConvertVolumeNametoVolumeId_)
{
    // Then
    int volumeId;
    int arrayId;
    std::string volumeName;
    int ret = posReplicatorManager->ConvertVolumeNametoVolumeId(volumeName, arrayId);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_AddDonePOSIoRequest_)
{
    // Then
    uint64_t lsn;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);

    EXPECT_CALL(*mockVolumeIo, GetArrayId()).WillOnce(Return(0));
    EXPECT_CALL(*mockVolumeIo, GetVolumeId()).WillOnce(Return(0));

    posReplicatorManager->AddDonePOSIoRequest(lsn, volumeIo);

    //delete mockVolumeIo;
}

} // namespace pos
