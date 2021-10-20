#include "src/bio/ubio.h"

#include <gtest/gtest.h>
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "src/include/core_const.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(Ubio, Constructor)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    Ubio* ubio = new Ubio(ptr, 8, 0);
    delete ubio;
    // Then : Nothing

    // Given : Temporary buffer pointer is given
    // When : Constructor and Destructor
    ptr = (void *)0x12345678;
    ubio = new Ubio(ptr, 8, 0);
    delete ubio;
    // Then : Nothing
}

TEST(Ubio, Split)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    Ubio* ubio = new Ubio(ptr, 16, 0);
    UbioSmartPtr ubioSplit = ubio->Split(8, false);
    ASSERT_EQ(ubio->GetSize() / 512, 8);
    ASSERT_EQ(ubio->GetMemSize() / 512, 16);
    delete ubio;
    // Then : Nothing
}

//We test SetSyncMode, SetAsyncMode, IsSyncMode, MarkDone.
TEST(Ubio, SyncRelatedFunction)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    Ubio* ubio = new Ubio(ptr, 16, 0);
    UblockSharedPtr ublock(new MockUBlockDevice("name", 8192, nullptr));
    // Then : Use normal case. 
    ubio->SetUblock(ublock);
    ubio->SetSyncMode();
    ASSERT_EQ(ubio->IsSyncMode(), true);
    ubio->MarkDone();
    ubio->WaitDone();
    delete ubio;

    // Given : Nothing
    // When : Constructor and Destructor
    ubio = new Ubio(ptr, 16, 0);
    // Use illegal case, even if we get error log, it should be not stuck.
    ubio->SetUblock(ublock);
    ubio->SetAsyncMode();
    ASSERT_EQ(ubio->IsSyncMode(), false);
    ubio->MarkDone();
    ubio->WaitDone();
    delete ubio;
}

//We test GetBuffer and GetWholeBuffer.
TEST(Ubio, GetBuffer)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void* ptr = nullptr;
    Ubio* ubio = new Ubio(ptr, 32, 0);

    // Then : specific offset given is well obtained as expected
    void* baseAddress = ubio->GetBuffer(1, 0);
    void* baseWholeAddress = ubio->GetWholeBuffer();

    ASSERT_EQ((void *)((char *)baseWholeAddress + 4096), baseAddress);

    baseAddress = ubio->GetBuffer(2, 3);
    ASSERT_EQ((void *)((char *)baseWholeAddress + 4096 * 2 + 512 * 3), baseAddress);

    delete ubio;
}

TEST(Ubio, Complete)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    UbioSmartPtr ubio(new Ubio(ptr, 32, 0));
    UblockSharedPtr ublock(new MockUBlockDevice("name", 8192, nullptr));
    ubio->Complete(IOErrorType::SUCCESS);
    UbioSmartPtr ubio2(new Ubio(ptr, 32, 0));
    ubio->WaitDone();
    // Then : Use normal case. 
    // Then : ubio error type is done and Mark is done
    ubio2->Complete(IOErrorType::ABORTED);
    ASSERT_EQ(ubio2->GetError(), IOErrorType::ABORTED);
    ubio->MarkDone();
}

TEST(Ubio, OriginRelated)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    UbioSmartPtr ubio(new Ubio(ptr, 32, 0));
    UbioSmartPtr ubio2(new Ubio(ptr, 32, 0));
    // Then : ubio origin set is properly done
    ubio->SetOriginUbio(ubio2);
    UbioSmartPtr ubio3 = ubio->GetOriginUbio();
    ASSERT_EQ(ubio2.get(), ubio3.get());
    // Then : ubio origin is properly cleared
    ubio->ClearOrigin();
    ubio3 = ubio->GetOriginUbio();
    ASSERT_EQ(nullptr, ubio3.get());
}

TEST(Ubio, SetPba)
{
    // Given : Nothing
    // When : Constructor and Destructor
    UbioSmartPtr ubio(new Ubio(nullptr, 32, 3));
    PhysicalBlkAddr pba, tempPba;
    pba.lba = 123;
    pba.arrayDev = nullptr;
    ubio->SetPba(pba);
    // Then : if array device is not set, lba is properly set as 0?
    ASSERT_EQ(ubio->GetLba(), 0);
    MockArrayDevice arrayDevice(nullptr, ArrayDeviceState::NORMAL);
    pba.arrayDev = &arrayDevice;
    ubio->SetPba(pba);
    tempPba = ubio->GetPba();
    // Then : get value after setting value is same?
    ASSERT_EQ(ubio->GetLba(), 123);
    ASSERT_EQ(ubio->GetArrayDev(), &arrayDevice);
    ASSERT_EQ(ubio->GetArrayId(), 3);
    ASSERT_EQ(tempPba.lba, 123);
}

TEST(Ubio, FreeDataBuffer)
{
    // Given : Nothing
    // When : Memory Alloc externally for a ubio
    void* ptr = Memory<4096>::Alloc();
    UbioSmartPtr ubio(new Ubio(nullptr, 32, 0));
    // Then : Nothing
    ubio->FreeDataBuffer();
}

class DummyCallback : public Callback
{
public:
    DummyCallback(bool isFront)
    : Callback(isFront)
    {
    }
    ~DummyCallback(void) override{};
private:
    bool
    _DoSpecificJob(void)
    {
        return true;
    }
};

TEST(Ubio, EventRelated)
{
    // Given : Nothing
    // When : Constructor and Destructor
    UbioSmartPtr ubio(new Ubio(nullptr, 32, 0));
    CallbackSmartPtr callback(new DummyCallback(true));
    PhysicalBlkAddr pba;
    pba.lba = 0;
    pba.arrayDev = nullptr;
    // Then : Nothing
    ubio->SetCallback(callback);
    ubio->SetEventType(BackendEvent_Flush);
    ASSERT_EQ(ubio->GetCallback().get(), callback.get());
    ASSERT_EQ(ubio->GetEventType(), BackendEvent_Flush);
}

TEST(Ubio, GetUBlock)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    Ubio* ubio = new Ubio(ptr, 16, 0);
    UblockSharedPtr ublock(new MockUBlockDevice("name", 8192, nullptr));
    // Then : Set ptr and check if pointers are same
    ubio->SetUblock(ublock);
    ASSERT_EQ(ubio->GetUBlock(), ublock.get());
}

TEST(Ubio, RetryRelated)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    Ubio* ubio = new Ubio(ptr, 16, 0);
    ubio->SetRetry(true);
    // Then : Use normal case. 
    ASSERT_EQ(ubio->IsRetry(), true);

    ubio->SetRetry(false);
    ASSERT_EQ(ubio->IsRetry(), false);
}

TEST(Ubio, ErrorRelated)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    UbioSmartPtr ubio(new Ubio(ptr, 32, 0));
    UblockSharedPtr ublock(new MockUBlockDevice("name", 8192, nullptr));
    // Then : Use normal case. 
    // Then : ubio error type is done and Mark is done
    ubio->SetError(IOErrorType::ABORTED);
    ASSERT_EQ(ubio->GetError(), IOErrorType::ABORTED);
    ubio->SetError(IOErrorType::SUCCESS);
    ASSERT_EQ(ubio->GetError(), IOErrorType::SUCCESS);
    ubio->ResetError();
    ASSERT_EQ(ubio->GetError(), IOErrorType::SUCCESS);
}

TEST(Ubio, GetOriginCore)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    UbioSmartPtr ubio(new Ubio(ptr, 32, 0));
    ASSERT_EQ(ubio->GetOriginCore(), INVALID_CORE);
}

TEST(Ubio, CheckRecoveryAllowed)
{
    // Given : Nothing
    // When : Constructor and Destructor
    void *ptr = nullptr;
    UbioSmartPtr ubio(new Ubio(ptr, 16, 0));
    // arrayDev == null, no allow to recovery
    ASSERT_EQ(ubio->CheckRecoveryAllowed(), false);
    MockArrayDevice arrayDevice(nullptr, ArrayDeviceState::NORMAL);
    PhysicalBlkAddr pba;
    pba.lba = 123;
    pba.arrayDev = &arrayDevice;
    // If arrayDev is set, success
    ubio->SetPba(pba);
    ASSERT_EQ(ubio->CheckRecoveryAllowed(), true);
    // If sync mode ubio, no allow to recovery
    ubio->SetSyncMode();
    ASSERT_EQ(ubio->CheckRecoveryAllowed(), false);
    
}

TEST(Ubio, SetReferenceIncreased)
{
}

TEST(Ubio, NeedRecovery_testIfIODoesNotGoToRecoveryWhenArrayDeviceIsNullptr)
{
    // Given
    Ubio ubio(nullptr, 0, 0);
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
    Ubio ubio(nullptr, 0, 0);
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
    Ubio ubio(nullptr, 0, 0);
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
    Ubio ubio(nullptr, 0, 0);
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
    Ubio ubio(nullptr, 0, 0);
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
    Ubio ubio(nullptr, 0, 0);
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
    Ubio ubio(nullptr, 0, 0);
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

TEST(Ubio, CheckPbaSet)
{
    // Given
    Ubio ubio(nullptr, 0, 0);
    EXPECT_EQ(ubio.CheckPbaSet(), false);
}

} // namespace pos
