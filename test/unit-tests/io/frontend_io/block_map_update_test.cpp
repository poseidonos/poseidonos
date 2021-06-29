#include "src/io/frontend_io/block_map_update.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/frontend_io/aio_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_completion_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/journal_service/i_journal_writer_mock.h"
#include "test/unit-tests/journal_service/journal_service_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(BlockMapUpdate, Constructor_Stack)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockMapperService> mockMapperService;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, ""));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIVSAMap> mockIVSAMap;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    EventSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        volumeIo, callback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    NiceMock<MockIVSAMap> vsaMap;

    ON_CALL(mockMapperService, GetIVSAMap(_)).WillByDefault(Return(&vsaMap));

    // When
    BlockMapUpdate blockMapUpdate(
        volumeIo, callback, []() -> bool { return false; },
        &vsaMap, &mockJournalService, &mockEventScheduler, mockBlockMapUpdateCompletionEvent);

    // Then : Do nothing
}

TEST(BlockMapUpdate, Constructor_Heap)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockMapperService> mockMapperService;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, ""));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIVSAMap> mockIVSAMap;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    EventSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        volumeIo, callback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    NiceMock<MockIVSAMap> vsaMap;

    ON_CALL(mockMapperService, GetIVSAMap(_)).WillByDefault(Return(&vsaMap));

    // When
    BlockMapUpdate* blockMapUpdate = new BlockMapUpdate(
        volumeIo, callback, []() -> bool { return false; },
        &vsaMap, &mockJournalService, &mockEventScheduler, mockBlockMapUpdateCompletionEvent);

    // Then : Do nothing

    delete blockMapUpdate;
}

TEST(BlockMapUpdate, Execute_Journal_Enabled)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockMapperService> mockMapperService;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockIJournalWriter> mockJournalWriter;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIVSAMap> mockIVSAMap;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    EventSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        volumeIo, callback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    MpageList mPageList;

    NiceMock<MockIVSAMap> vsaMap;

    // When : journal is enabled
    ON_CALL(mockMapperService, GetIVSAMap(_)).WillByDefault(Return(&vsaMap));
    ON_CALL(mockJournalService, IsEnabled(_)).WillByDefault(Return(true));
    ON_CALL(mockJournalService, GetWriter(_)).WillByDefault(Return(&mockJournalWriter));
    ON_CALL(vsaMap, GetDirtyVsaMapPages(_, _, _)).WillByDefault(Return(mPageList));
    ON_CALL(mockJournalWriter, AddBlockMapUpdatedLog(_, _, _)).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(1));

    BlockMapUpdate blockMapUpdate(
        volumeIo, callback, []() -> bool { return false; },
        &vsaMap, &mockJournalService, &mockEventScheduler, mockBlockMapUpdateCompletionEvent);

    // Then : Execute
    bool actual = blockMapUpdate.Execute();
    bool expected = true;
    volumeIo = nullptr;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdate, Execute_Journal_Enabled_Fail)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockMapperService> mockMapperService;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockIJournalWriter> mockJournalWriter;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIVSAMap> mockIVSAMap;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    EventSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        volumeIo, callback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    MpageList mPageList;

    NiceMock<MockIVSAMap> vsaMap;

    // When : journal is enabled
    ON_CALL(mockMapperService, GetIVSAMap(_)).WillByDefault(Return(&vsaMap));
    ON_CALL(mockJournalService, IsEnabled(_)).WillByDefault(Return(true));
    ON_CALL(mockJournalService, GetWriter(_)).WillByDefault(Return(&mockJournalWriter));
    ON_CALL(vsaMap, GetDirtyVsaMapPages(_, _, _)).WillByDefault(Return(mPageList));
    ON_CALL(mockJournalWriter, AddBlockMapUpdatedLog(_, _, _)).WillByDefault(Return(-1));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(1));

    BlockMapUpdate blockMapUpdate(
        volumeIo, callback, []() -> bool { return false; },
        &vsaMap, &mockJournalService, &mockEventScheduler, mockBlockMapUpdateCompletionEvent);

    // Then : Execute
    bool actual = blockMapUpdate.Execute();
    bool expected = false;
    ASSERT_EQ(actual, expected);

    // Then 2 : Execute the failed event again
    actual = blockMapUpdate.Execute();
    expected = false;
    volumeIo = nullptr;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdate, Execute_Journal_Not_Enabled)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockMapperService> mockMapperService;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockIJournalWriter> mockJournalWriter;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIVSAMap> mockIVSAMap;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    MockBlockMapUpdateCompletion* mockBlockMapUpdateCompletion = new NiceMock<MockBlockMapUpdateCompletion>(
        volumeIo, callback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    EventSmartPtr mockBlockMapUpdateCompletionEvent(mockBlockMapUpdateCompletion);

    // When : journal is not enabled
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(mockMapperService, GetIVSAMap(_)).WillByDefault(Return(&mockIVSAMap));
    ON_CALL(mockJournalService, IsEnabled(_)).WillByDefault(Return(false));
    ON_CALL(*mockBlockMapUpdateCompletion, Execute()).WillByDefault(Return(true));

    BlockMapUpdate blockMapUpdate(
        volumeIo, callback, []() -> bool { return false; },
        &mockIVSAMap, &mockJournalService, &mockEventScheduler, mockBlockMapUpdateCompletionEvent);

    // Then : Execute
    bool actual = blockMapUpdate.Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdate, Execute_Journal_Not_Enabled_Fail)
{
    // Given
    const uint32_t unitCount = 8;
    NiceMock<MockMapperService> mockMapperService;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockIJournalWriter> mockJournalWriter;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIVSAMap> mockIVSAMap;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    MockBlockMapUpdateCompletion* mockBlockMapUpdateCompletion = new NiceMock<MockBlockMapUpdateCompletion>(
        volumeIo, callback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    EventSmartPtr mockBlockMapUpdateCompletionEvent(mockBlockMapUpdateCompletion);

    ON_CALL(mockJournalService, IsEnabled(_)).WillByDefault(Return(false));
    ON_CALL(*mockBlockMapUpdateCompletion, Execute()).WillByDefault(Return(false));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    // When
    BlockMapUpdate blockMapUpdateFalse(
        volumeIo, callback, []() -> bool { return false; },
        &mockIVSAMap, &mockJournalService, &mockEventScheduler, mockBlockMapUpdateCompletionEvent);

    // Then : Execute When jounal is off
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);
    bool actual = blockMapUpdateFalse.Execute();

    bool expected = true;
    volumeIo = nullptr;
    ASSERT_EQ(actual, expected);
}

} // namespace pos
