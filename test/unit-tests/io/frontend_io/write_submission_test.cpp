#include "src/io/frontend_io/write_submission.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/bio/volume_io.h"

#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/allocator_service/allocator_service_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(WriteSubmission, WriteSubmission_Constructor_One)
{
    // Given
    char buf[1024];
    std::string arr_name = "";

    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 512 >> SECTOR_SIZE_SHIFT, arr_name);
    volumeIo->SetSectorRba(2048);
    volumeIo->SetVolumeId(1);

    // when
    WriteSubmission writeSubmission(volumeIo);

    // Then : do noting
}

TEST(WriteSubmission, WriteSubmission_Constructor_Three)
{
    // Given
    char buf[1024];
    std::string arr_name = "";

    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 512 >> SECTOR_SIZE_SHIFT, arr_name);
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(0);
    
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;

    // when
    WriteSubmission writeSubmission(volumeIo, &mockRBAStateManager, &mockIBlockAllocator);

    // Then : do noting
}

TEST(WriteSubmission, Execute_SingleBlock_ownershipFail)
{
    // Given
    char buf[1024];
    std::string arr_name = "";

    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 512 >> SECTOR_SIZE_SHIFT, arr_name);
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(0);

    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockRBAStateManager> mockRBAStateManager("");
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;

    // when
    WriteSubmission writeSubmission(volumeIo, &mockRBAStateManager, &mockIBlockAllocator);
    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(false));

    bool actual, expected{false};

    // Then
    actual = writeSubmission.Execute();

    ASSERT_EQ(expected, actual);
}

TEST(WriteSubmission, Execute_SingleBlock)
{
    // Given
    char buf[512];
    std::string arr_name = "";

    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;

    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, arr_name));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    mockVolumeIo->SetCallback(callback);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 1};

    // when
    WriteSubmission writeSubmission(mockVolumeIo, &mockRBAStateManager, &mockIBlockAllocator);

    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(vsaRange));

    bool actual, expected{true};

    // Then
    actual = writeSubmission.Execute();

    ASSERT_EQ(expected, actual);
}

TEST(WriteSubmission, Execute_AlgnedMultiBlock)
{
    // Given
    char buf[4096];
    std::string arr_name = "";

    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;

    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, arr_name));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    mockVolumeIo->SetCallback(callback);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 8};

    // when
    WriteSubmission writeSubmission(mockVolumeIo, &mockRBAStateManager, &mockIBlockAllocator);

    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(vsaRange));

    bool actual, expected{true};

    // Then
    actual = writeSubmission.Execute();

    ASSERT_EQ(expected, actual);
}

TEST(WriteSubmission, Execute_MisAlgnedMultiBlock)
{
    // Given
    char buf[4096];
    std::string arr_name = "";

    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;

    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, arr_name));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    mockVolumeIo->SetCallback(callback);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 4};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 8};

    // when
    WriteSubmission writeSubmission(mockVolumeIo, &mockRBAStateManager, &mockIBlockAllocator);

    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(vsaRange));

    bool actual, expected{true};

    // Then
    actual = writeSubmission.Execute();

    ASSERT_EQ(expected, actual);
}

} // namespace pos
