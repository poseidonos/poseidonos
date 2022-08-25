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
#include "src/pos_replicator/dummy_ha/dummy_ha_client.h"
#include "src/pos_replicator/dummy_ha/dummy_ha_server.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/pos_replicator/grpc_publisher_mock.h"
#include "test/unit-tests/pos_replicator/grpc_subscriber_mock.h"

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

    DummyHaServer* haServer;
    DummyHaClient* haClient;
    NiceMock<MockConfigManager>* configManager;
    PosReplicatorManager* posReplicatorManager;
    NiceMock<MockGrpcPublisher>* grpcPublisher;
    NiceMock<MockGrpcSubscriber>* grpcSubscriber;
};

void
PosReplicatorManagerTestFixture::SetUp(void)
{
    // new Server : HA side
    haServer = new DummyHaServer();
    string serverAddress(GRPC_HA_PUB_SERVER_SOCKET_ADDRESS);
    new std::thread(&DummyHaServer::RunServer, haServer, serverAddress);
    sleep(1);

    // new PosReplicator : POS side
    configManager = new NiceMock<MockConfigManager>;
    ON_CALL(*configManager, GetValue("replicator", "ha_publisher_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50003"));
    ON_CALL(*configManager, GetValue("replicator", "ha_subscriber_address", _, _)).WillByDefault(SetArg2ToStringAndReturn0("0.0.0.0:50053"));

    posReplicatorManager = new PosReplicatorManager();
    grpcPublisher = new NiceMock<MockGrpcPublisher>(nullptr, configManager);
    grpcSubscriber = new NiceMock<MockGrpcSubscriber>(configManager);
    posReplicatorManager->Init(grpcPublisher, grpcSubscriber);
    sleep(1);

    // new Client : HA side
    haClient = new DummyHaClient(nullptr);
}

void
PosReplicatorManagerTestFixture::TearDown(void)
{
    delete configManager;
    delete haServer;
    delete haClient;
    posReplicatorManager->Dispose();
    delete posReplicatorManager;
    sleep(1);
}

TEST_F(PosReplicatorManagerTestFixture, NotifyNewUserIORequest_)
{
    // Given

    pos_io io;
    io.ioType = IO_TYPE::WRITE;
    io.array_id = 0;
    io.volume_id = 0;
    io.arrayName = "";

    // Then
    int ret = posReplicatorManager->NotifyNewUserIORequest(io);

    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, CompelteUserIO_)
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    int ret = posReplicatorManager->CompleteUserIO(lsn, arrayId, volumeId);

    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, UserVolumeWriteSubmission_) // To do
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    int ret = posReplicatorManager->UserVolumeWriteSubmission(lsn, arrayId, volumeId);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, DISABLED_HAIOSubmission_) // To do
{
    // Given

    // Then
    IO_TYPE ioType;
    int arrayId = 0;
    int volumeId = 0;
    uint64_t rba;
    uint64_t num_blocks;
    void* data;

    int ret = posReplicatorManager->HAIOSubmission(ioType, arrayId, volumeId, rba, num_blocks, data);
    EXPECT_EQ(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManagerTestFixture, HAIOCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    pos_io io;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);

    posReplicatorManager->HAIOCompletion(lsn, io, volumeIo);
}

TEST_F(PosReplicatorManagerTestFixture, HAWriteCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    pos_io io;
    posReplicatorManager->HAWriteCompletion(lsn, io);
}

TEST_F(PosReplicatorManagerTestFixture, HAReadCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    pos_io io;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);

    posReplicatorManager->HAReadCompletion(lsn, io, volumeIo);
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

TEST_F(PosReplicatorManagerTestFixture, AddDonePOSIoRequest_)
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
