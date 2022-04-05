#include "src/io/frontend_io/write_submission.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/array_mgmt/array_manager.h"
#include "src/bio/volume_io.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator_service/allocator_service_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/gc/flow_control/flow_control_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Matcher;

namespace pos
{
TEST(WriteSubmission, WriteSubmission_Stack)
{
    // Given
    char buf[1024];
    std::string arr_name = "";

    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 512 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(0);

    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name, 0);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockFlowControl> mockFlowControl(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockIArrayInfo> mockArrayInfo;

    // when
    WriteSubmission writeSubmission(volumeIo, &mockRBAStateManager, &mockIBlockAllocator,
        &mockFlowControl, &mockArrayInfo, false);

    // Then : do noting
}

TEST(WriteSubmission, WriteSubmission_Heap)
{
    // Given
    char buf[1024];
    std::string arr_name = "";

    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 512 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(0);

    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name, 0);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockFlowControl> mockFlowControl(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockIArrayInfo> mockArrayInfo;

    // when : create write submission
    WriteSubmission* writeSubmission = new WriteSubmission(volumeIo, &mockRBAStateManager,
        &mockIBlockAllocator, &mockFlowControl, &mockArrayInfo, false);

    // Then : delete write submission
    delete writeSubmission;
}

TEST(WriteSubmission, Execute_SingleBlock_ownershipFail)
{
    // Given
    char buf[1024];
    std::string arr_name = "";

    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 512 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(0);

    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockRBAStateManager> mockRBAStateManager("", 0);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockFlowControl> mockFlowControl(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockIArrayInfo> mockArrayInfo;

    // when
    WriteSubmission writeSubmission(volumeIo, &mockRBAStateManager, &mockIBlockAllocator,
        &mockFlowControl, &mockArrayInfo, false);

    ON_CALL(mockFlowControl, GetToken(_, _)).WillByDefault(Return((512 >> SECTOR_SIZE_SHIFT) * Ubio::BYTES_PER_UNIT));
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
    NiceMock<MockRBAStateManager> mockRBAStateManager("", 0);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockFlowControl> mockFlowControl(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockIArrayInfo> mockArrayInfo;
    ON_CALL(mockArrayInfo, IsWriteThroughEnabled()).WillByDefault(Return(false));

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    ON_CALL(*mockVolumeIo, GetArrayId()).WillByDefault(Return(0));

    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    volumeIo->SetCallback(callback);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 1};

    // when
    WriteSubmission writeSubmission(volumeIo, &mockRBAStateManager, &mockIBlockAllocator,
        &mockFlowControl, &mockArrayInfo, false);

    ON_CALL(mockFlowControl, GetToken(_, _)).WillByDefault(Return((512 >> SECTOR_SIZE_SHIFT) * Ubio::BYTES_PER_UNIT));
    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(std::make_pair(vsaRange, UNMAP_STRIPE)));

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
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name, 0);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockFlowControl> mockFlowControl(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockIArrayInfo> mockArrayInfo;

    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    mockVolumeIo->SetCallback(callback);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 8};

    // when
    WriteSubmission writeSubmission(mockVolumeIo, &mockRBAStateManager, &mockIBlockAllocator,
        &mockFlowControl, &mockArrayInfo, false);

    ON_CALL(mockFlowControl, GetToken(_, _)).WillByDefault(Return((512 >> SECTOR_SIZE_SHIFT) * Ubio::BYTES_PER_UNIT));
    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(std::make_pair(vsaRange, UNMAP_STRIPE)));

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
    NiceMock<MockRBAStateManager> mockRBAStateManager(arr_name, 0);
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockFlowControl> mockFlowControl(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockIArrayInfo> mockArrayInfo;

    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    mockVolumeIo->SetCallback(callback);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 4};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 8};

    // when
    WriteSubmission writeSubmission(mockVolumeIo, &mockRBAStateManager, &mockIBlockAllocator,
        &mockFlowControl, &mockArrayInfo, false);

    ON_CALL(mockFlowControl, GetToken(_, _)).WillByDefault(Return((512 >> SECTOR_SIZE_SHIFT) * Ubio::BYTES_PER_UNIT));
    ON_CALL(mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(std::make_pair(vsaRange, UNMAP_STRIPE)));

    bool actual, expected{true};

    // Then
    actual = writeSubmission.Execute();

    ASSERT_EQ(expected, actual);
}

} // namespace pos
