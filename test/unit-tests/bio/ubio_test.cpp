#include "src/bio/ubio.h"

#include <gtest/gtest.h>
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(Ubio, Ubio_)
{
}

TEST(Ubio, Split_)
{
}

TEST(Ubio, MarkDone_)
{
}

TEST(Ubio, GetBuffer_)
{
}

TEST(Ubio, GetWholeBuffer_)
{
}

TEST(Ubio, WaitDone_)
{
}

TEST(Ubio, Complete_)
{
}

TEST(Ubio, ClearOrigin_)
{
}

TEST(Ubio, SetPba_)
{
}

TEST(Ubio, FreeDataBuffer_)
{
}

TEST(Ubio, SetSyncMode_)
{
}

TEST(Ubio, SetAsyncMode_)
{
}

TEST(Ubio, SetCallback_)
{
}

TEST(Ubio, GetCallback_)
{
}

TEST(Ubio, ClearCallback_)
{
}

TEST(Ubio, GetUBlock_)
{
}

TEST(Ubio, GetArrayDev_)
{
}

TEST(Ubio, GetLba_)
{
}

TEST(Ubio, GetPba_)
{
}

TEST(Ubio, SetRba_)
{
}

TEST(Ubio, GetRba_)
{
}

TEST(Ubio, IsRetry_)
{
}

TEST(Ubio, SetRetry_)
{
}

TEST(Ubio, GetMemSize_)
{
}

TEST(Ubio, GetOriginCore_)
{
}

TEST(Ubio, SetOriginUbio_)
{
}

TEST(Ubio, GetOriginUbio_)
{
}

TEST(Ubio, GetSize_)
{
}

TEST(Ubio, GetError_)
{
}

TEST(Ubio, SetError_)
{
}

TEST(Ubio, ResetError_)
{
}

TEST(Ubio, CheckPbaSet_)
{
}

TEST(Ubio, CheckRecoveryAllowed_)
{
}

TEST(Ubio, SetReferenceIncreased_)
{
}

TEST(Ubio, IsSyncMode_)
{
}

TEST(Ubio, SetLba_)
{
}

TEST(Ubio, SetUblock_)
{
}

TEST(Ubio, NeedRecovery_testIfIODoesNotGoToRecoveryWhenArrayDeviceIsNullptr)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = nullptr;
    ubio.SetPba(pba);
    // When
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_FALSE(res);
}

TEST(Ubio, NeedRecovery_testIfIODoesNotGoToRecoveryWhenIOIsSyncMode)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    PhysicalBlkAddr pba;
    MockArrayDevice* dev = new MockArrayDevice(nullptr);
    pba.lba = 0;
    pba.arrayDev = dev;
    ubio.SetPba(pba);
    EXPECT_CALL(*dev, GetUblock).WillOnce(Return(nullptr));
    // When
    ubio.SetSyncMode();
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_FALSE(res);

    // Cleanup
    delete dev;
}

TEST(Ubio, NeedRecovery_testIfIOGoesToRecoveryWhenArrayDeviceIsFault)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    MockArrayDevice* dev = new MockArrayDevice(nullptr);
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = dev;
    ubio.SetPba(pba);
    EXPECT_CALL(*dev, GetState).WillOnce(Return(ArrayDeviceState::FAULT));
    // When
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_TRUE(res);

    // Cleanup
    delete dev;
}

TEST(Ubio, NeedRecovery_testIfIOGoesToRecoveryWhenReadingOnRebuildingDevice)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    MockArrayDevice* dev = new MockArrayDevice(nullptr);
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = dev;
    ubio.SetPba(pba);
    EXPECT_CALL(*dev, GetState).WillOnce(Return(ArrayDeviceState::REBUILD));
    // When
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_TRUE(res);

    // Cleanup
    delete dev;
}

TEST(Ubio, NeedRecovery_testIfIODoesNotGoToRecoveryWhenWritingOnRebuildingDevice)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    MockUBlockDevice *ublock = new MockUBlockDevice("", 0, nullptr);
    UblockSharedPtr devSharedPtr(ublock);
    MockArrayDevice* dev = new MockArrayDevice(nullptr);
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = dev;
    ubio.SetPba(pba);
    EXPECT_CALL(*dev, GetState).WillOnce(Return(ArrayDeviceState::REBUILD));
    EXPECT_CALL(*dev, GetUblock).WillOnce(Return(devSharedPtr));
    // When
    ubio.dir = UbioDir::Write;
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_FALSE(res);

    // Cleanup
    delete dev;
}

TEST(Ubio, NeedRecovery_testIfIODoesNotGoToRecoveryWhenTrimming)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    MockUBlockDevice *ublock = new MockUBlockDevice("", 0, nullptr);
    UblockSharedPtr devSharedPtr(ublock);
    MockArrayDevice* dev = new MockArrayDevice(nullptr);
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = dev;
    ubio.SetPba(pba);
    EXPECT_CALL(*dev, GetState).WillOnce(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*dev, GetUblock).WillOnce(Return(devSharedPtr));
    // When
    ubio.dir = UbioDir::Deallocate;
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_FALSE(res);

    // Cleanup
    delete dev;
}

TEST(Ubio, NeedRecovery_testIfIODoesNotGoToRecoveryWhenReadingOnNormalDevice)
{
    // Given
    Ubio ubio(nullptr, 0, "mock-array");
    MockUBlockDevice *ublock = new MockUBlockDevice("", 0, nullptr);
    UblockSharedPtr devSharedPtr(ublock);
    MockArrayDevice* dev = new MockArrayDevice(nullptr);
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = dev;
    ubio.SetPba(pba);
    EXPECT_CALL(*dev, GetState).WillOnce(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*dev, GetUblock).WillOnce(Return(devSharedPtr));
    // When
    ubio.dir = UbioDir::Read;
    int res = ubio.NeedRecovery();
    // Then
    ASSERT_FALSE(res);

    // Cleanup
    delete dev;
}

TEST(Ubio, GetUbioSize_)
{
}

TEST(Ubio, SetEventType_)
{
}

TEST(Ubio, GetEventType_)
{
}

TEST(Ubio, _ReflectSplit_)
{
}

TEST(Ubio, CheckOriginUbioSet_)
{
}

TEST(Ubio, Advance_)
{
}

TEST(Ubio, Retreat_)
{
}

} // namespace pos
