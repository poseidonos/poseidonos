#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

#include "spdk/pos.h"
#include "src/pos_replicator/posreplicator_manager.h"
#include "src/pos_replicator/dummy_ha/dummy_ha_server.h"
#include "src/pos_replicator/dummy_ha/dummy_ha_client.h"
#include "test/unit-tests/bio/volume_io_mock.h"

using namespace ::testing;

namespace pos
{

class PosReplicatorManager_ut : public ::testing::Test
{
protected:
    void SetUp(void) override;
    void TearDown(void) override;

    DummyHaServer *haServer;
    DummyHaClient *haClient;
    PosReplicatorManager* posReplicatorManager;
};


void
PosReplicatorManager_ut::SetUp(void)
{
    // new Server : HA side
    haServer = new DummyHaServer();
    usleep(1000);

    // new PosReplicator : POS side
    posReplicatorManager = new PosReplicatorManager();
    posReplicatorManager->Init();
    usleep(1000);
    
    // new Client : HA side
    haClient = new DummyHaClient(nullptr);    
}

void
PosReplicatorManager_ut::TearDown(void)
{
    delete haServer;    
    delete haClient;

    delete posReplicatorManager;
}

TEST_F(PosReplicatorManager_ut, Init_)
{
    // Then
    posReplicatorManager->Init();
    usleep(1000);

    posReplicatorManager->Dispose();
}

TEST_F(PosReplicatorManager_ut, NotifyNewUserIORequest_)
{
    // Given

    pos_io io;
    io.ioType = IO_TYPE::WRITE;
    io.array_id = 0;
    io.volume_id = 0;
    io.arrayName = "";

    // Then
    posReplicatorManager->NotifyNewUserIORequest(io);
    
}

TEST_F(PosReplicatorManager_ut, CompelteUserIO_)
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    posReplicatorManager->CompelteUserIO(lsn, arrayId, volumeId);

}

TEST_F(PosReplicatorManager_ut, UserVolumeWriteSubmission_) // To do
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    posReplicatorManager->UserVolumeWriteSubmission(lsn, arrayId, volumeId);
}

TEST_F(PosReplicatorManager_ut, HAIOSubmission_) // To do
{
    // Given

    // Then
    IO_TYPE ioType;
    int arrayId = 0;
    int volumeId = 0;
    uint64_t rba;
    uint64_t num_blocks;
    void* data;

    //posReplicatorManager->HAIOSubmission(ioType, arrayId, volumeId, rba, num_blocks, data);
}

TEST_F(PosReplicatorManager_ut, HAIOCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    pos_io io;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);

    posReplicatorManager->HAIOCompletion(lsn, io, volumeIo);
}

TEST_F(PosReplicatorManager_ut, HAWriteCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    pos_io io;
    posReplicatorManager->HAWriteCompletion(lsn, io);
}

TEST_F(PosReplicatorManager_ut, HAReadCompletion_)
{
    // Given

    // Then
    uint64_t lsn;
    pos_io io;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);

    posReplicatorManager->HAReadCompletion(lsn, io, volumeIo);
}

TEST_F(PosReplicatorManager_ut, ConvertArrayIdtoArrayName_)
{
    // Given

    // Then
    int arrayId;
    string arrayName;
    posReplicatorManager->ConvertArrayIdtoArrayName(arrayId, arrayName);
}

TEST_F(PosReplicatorManager_ut, ConvertVolumeIdtoVolumeName_)
{
    // Given
    int volumeId;
    int arrayId;
    string volumeName;

    // Then
    posReplicatorManager->ConvertVolumeIdtoVolumeName(volumeId, arrayId, volumeName);
}

TEST_F(PosReplicatorManager_ut, ConvertArrayNametoArrayId_)
{
    // Then
    int volumeId;
    int arrayId;
    std::string arrayName;
    posReplicatorManager->ConvertArrayNametoArrayId(arrayName);
}

TEST_F(PosReplicatorManager_ut, ConvertVolumeNametoVolumeId_)
{
    // Then
    int volumeId;
    int arrayId;
    std::string volumeName;
    posReplicatorManager->ConvertVolumeNametoVolumeId(volumeName, arrayId);
}

TEST_F(PosReplicatorManager_ut, AddDonePOSIoRequest_)
{
    // Then
    uint64_t lsn;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);

    EXPECT_CALL(*mockVolumeIo, GetArrayId()).WillOnce(Return(0));
    EXPECT_CALL(*mockVolumeIo, GetVolumeId()).WillOnce(Return(0));

    posReplicatorManager->AddDonePOSIoRequest(lsn, volumeIo);

    //delete mockVolumeIo;
}

}