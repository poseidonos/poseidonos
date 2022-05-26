#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

#include "spdk/pos.h"
#include "src/include/pos_event_id.h"
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
    int ret = posReplicatorManager->NotifyNewUserIORequest(io);

    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManager_ut, CompelteUserIO_)
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    int ret = posReplicatorManager->CompleteUserIO(lsn, arrayId, volumeId);

    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManager_ut, UserVolumeWriteSubmission_) // To do
{
    // Given

    // Then
    uint64_t lsn = 0;
    int arrayId = 0;
    int volumeId = 0;

    int ret = posReplicatorManager->UserVolumeWriteSubmission(lsn, arrayId, volumeId);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManager_ut, DISABLED_HAIOSubmission_) // To do
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
    int ret = posReplicatorManager->ConvertArrayIdtoArrayName(arrayId, arrayName);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManager_ut, ConvertVolumeIdtoVolumeName_)
{
    // Given
    int volumeId;
    int arrayId;
    string volumeName;

    // Then
    int ret = posReplicatorManager->ConvertVolumeIdtoVolumeName(volumeId, arrayId, volumeName);
}

TEST_F(PosReplicatorManager_ut, ConvertArrayNametoArrayId_)
{
    // Then
    int volumeId;
    int arrayId;
    std::string arrayName;
    int ret = posReplicatorManager->ConvertArrayNametoArrayId(arrayName);
    EXPECT_NE(EID(SUCCESS), ret);
}

TEST_F(PosReplicatorManager_ut, ConvertVolumeNametoVolumeId_)
{
    // Then
    int volumeId;
    int arrayId;
    std::string volumeName;
    int ret = posReplicatorManager->ConvertVolumeNametoVolumeId(volumeName, arrayId);
    EXPECT_NE(EID(SUCCESS), ret);
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