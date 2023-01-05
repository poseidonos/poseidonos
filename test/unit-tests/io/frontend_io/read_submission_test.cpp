#include "src/io/frontend_io/read_submission.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "spdk/event.h"
#include "src/array/array.h"
#include "src/array/device/array_device.h"
#include "src/bio/volume_io.h"
#include "src/device/base/ublock_device.h"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"
#include "src/event_scheduler/callback.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/read_completion.h"
#include "src/io/general_io/io_controller.h"
#include "src/io/general_io/io_recovery_event_factory.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/mapper_service/mapper_service.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/device/base/device_driver_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/include/i_array_device_mock.h"
#include "test/unit-tests/io/general_io/merger_mock.h"
#include "test/unit-tests/io/general_io/translator_mock.h"
#include "test/unit-tests/lib/block_alignment_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

TEST(ReadSubmission, ReadSubmission_Stack)
{
    // Given
    char buf[1024];

    std::string arr_name = "";
    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 1024 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(2048);
    volumeIo->SetVolumeId(1);
    NiceMock<MockBlockAlignment>* mockBlockAlignment = new NiceMock<MockBlockAlignment>{0, 0};
    ReadCompletionFactory readCompletionFactory;
    NiceMock<MockMerger>* mockMerger = new NiceMock<MockMerger>{volumeIo, &readCompletionFactory};
    BlkAddr blkAddr;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVolumeInfoManager> mockVolumeInfoManager;

    MockTranslator* mockTranslator = new MockTranslator(1, blkAddr, 0, 0, true, &mockIVSAMap, &mockIStripeMap, &mockIWBStripeAllocator, nullptr, &mockVolumeInfoManager);

    // When: Try to create new ReadSubmission object with 4 arguments
    ReadSubmission readSubmission{volumeIo, mockBlockAlignment, mockMerger, mockTranslator};

    // Then: Do nothing
}

TEST(ReadSubmission, ReadSubmission_Heap)
{
    // Given
    char buf[1024];
    std::string arr_name = "";
    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 1024 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(2048);
    volumeIo->SetVolumeId(1);
    NiceMock<MockBlockAlignment>* mockBlockAlignment = new NiceMock<MockBlockAlignment>{0, 0};
    ReadCompletionFactory readCompletionFactory;
    NiceMock<MockMerger>* mockMerger = new NiceMock<MockMerger>{volumeIo, &readCompletionFactory};
    BlkAddr blkAddr;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVolumeInfoManager> mockVolumeInfoManager;

    MockTranslator* mockTranslator = new MockTranslator(1, blkAddr, 0, 0, true, &mockIVSAMap, &mockIStripeMap, &mockIWBStripeAllocator, nullptr, &mockVolumeInfoManager);

    // When: Try to create new ReadSubmission object with 4 arguments
    ReadSubmission* readSubmission = new ReadSubmission{volumeIo, mockBlockAlignment, mockMerger, mockTranslator};

    // Then: Release memory
    delete readSubmission;
    readSubmission = nullptr;
}

TEST(ReadSubmission, Execute_SingleBlock)
{
    // Given
    char buf[1024];
    std::string arr_name = "";
    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 1024 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(2048);
    volumeIo->SetVolumeId(1);
    CallbackSmartPtr callback = std::make_shared<ReadCompletion>(volumeIo);
    volumeIo->SetCallback(callback);
    NiceMock<MockBlockAlignment>* mockBlockAlignment = new NiceMock<MockBlockAlignment>{0, 0};
    ReadCompletionFactory readCompletionFactory;
    NiceMock<MockMerger>* mockMerger = new NiceMock<MockMerger>{volumeIo, &readCompletionFactory};
    BlkAddr blkAddr;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVolumeInfoManager> mockVolumeInfoManager;

    MockTranslator* mockTranslator = new MockTranslator(1, blkAddr, 0, 0, true, &mockIVSAMap, &mockIStripeMap, &mockIWBStripeAllocator, nullptr, &mockVolumeInfoManager);
    ReadSubmission readSubmission{volumeIo, mockBlockAlignment, mockMerger, mockTranslator};

    ON_CALL(*mockBlockAlignment, GetBlockCount()).WillByDefault(Return(1));
    MockIArrayDevice mockIArrayDevice;
    NiceMock<MockDeviceDriver> mockDeviceDriver;
    UblockSharedPtr mockUblock = std::make_shared<NiceMock<MockUBlockDevice>>("test", 4096, &mockDeviceDriver);
    ON_CALL(mockIArrayDevice, GetUblock).WillByDefault(Return(mockUblock));
    PhysicalBlkAddr physicalBlkAddr{0, &mockIArrayDevice};
    ON_CALL(*mockTranslator, GetPba()).WillByDefault(Return(physicalBlkAddr));
    EXPECT_CALL(*mockTranslator, GetPba()).Times(1);
    ON_CALL(*mockBlockAlignment, AlignHeadLba(_, _)).WillByDefault(Return(1));
    StripeAddr stripeAddr;
    stripeAddr.stripeId = 123;
    auto lsidRefResult = std::make_tuple(stripeAddr, true);
    ON_CALL(*mockTranslator, GetLsidRefResult(_)).WillByDefault(Return(lsidRefResult));
    EXPECT_CALL(*mockTranslator, GetLsidRefResult(_)).Times(1);

    bool actual, expected{true};
    uint64_t actualStripeId, expectedStripeId{123};

    // When: Call Execute(), _PrepareSingleBlock set some data
    actual = readSubmission.Execute();
    actualStripeId = volumeIo->GetLsidEntry().stripeId;

    // Then: Return true
    ASSERT_EQ(expected, actual);
    ASSERT_EQ(expectedStripeId, actualStripeId);
}

TEST(ReadSubmission, Execute_MultiBlocks)
{
    // Given
    char buf[1024];
    std::string arr_name = "";
    VolumeIoSmartPtr volumeIo = std::make_shared<VolumeIo>((void*)buf, 1024 >> SECTOR_SIZE_SHIFT, 0);
    volumeIo->SetSectorRba(2048);
    volumeIo->SetVolumeId(1);
    NiceMock<MockDeviceDriver> mockDeviceDriver;
    UblockSharedPtr block = std::make_shared<NiceMock<MockUBlockDevice>>("test", 4096, &mockDeviceDriver);
    volumeIo->SetUblock(block);
    CallbackSmartPtr callback = std::make_shared<ReadCompletion>(volumeIo);
    volumeIo->SetCallback(callback);
    NiceMock<MockBlockAlignment>* mockBlockAlignment = new NiceMock<MockBlockAlignment>{0, 0};
    ReadCompletionFactory readCompletionFactory;
    NiceMock<MockMerger>* mockMerger = new NiceMock<MockMerger>{volumeIo, &readCompletionFactory};
    BlkAddr blkAddr;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVolumeInfoManager> mockVolumeInfoManager;
    MockTranslator* mockTranslator = new MockTranslator(1, blkAddr, 0, 0, true, &mockIVSAMap, &mockIStripeMap, &mockIWBStripeAllocator, nullptr, &mockVolumeInfoManager);
    ReadSubmission readSubmission{volumeIo, mockBlockAlignment, mockMerger, mockTranslator};

    ON_CALL(*mockBlockAlignment, GetBlockCount()).WillByDefault(Return(2));
    EXPECT_CALL(*mockTranslator, GetPba(_)).Times(2);
    EXPECT_CALL(*mockTranslator, GetVsa(_)).Times(2);
    EXPECT_CALL(*mockTranslator, GetLsidEntry(_)).Times(2);
    ON_CALL(*mockBlockAlignment, AlignHeadLba(_, _)).WillByDefault(Return(1));
    ON_CALL(*mockMerger, Cut()).WillByDefault(Return());
    ON_CALL(*mockMerger, GetSplitCount()).WillByDefault(Return(2));
    ON_CALL(*mockMerger, GetSplit(_)).WillByDefault(Return(volumeIo));

    bool actual, expected{true};

    // When: Call Execute()
    actual = readSubmission.Execute();

    // Then: Return true
    ASSERT_EQ(expected, actual);
}

} // namespace pos
