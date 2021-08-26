#include "src/metadata/meta_updater.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/journal_service/i_journal_writer_mock.h"
#include "test/unit-tests/journal_service/i_journal_manager_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(MetaUpdater, MetaUpdater_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;

    // When
    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler);

    // Then : Do nothing
}

TEST(MetaUpdater, UpdateBlockMap_testIfExecutedSuccessullfyWhenJournalEnabled)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler);

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    NiceMock<MockCallback>* mockCallback = new NiceMock<MockCallback>(true);
    CallbackSmartPtr callback(mockCallback);
    MpageList dirtyList;

    ON_CALL(journal, IsEnabled).WillByDefault(Return(true));
    ON_CALL(vsaMap, GetDirtyVsaMapPages).WillByDefault(Return(dirtyList));

    EXPECT_CALL(journalWriter, AddBlockMapUpdatedLog(volumeIo, dirtyList, _)).WillOnce(Return(0));

    int expected = 0;
    int actual = metaUpdater.UpdateBlockMap(volumeIo, callback);

    EXPECT_EQ(actual, expected);
}

TEST(BlockMapUpdate, UpdateBlockMap_testIfFailsWhenJournalWriteFails)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler);

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    NiceMock<MockCallback>* mockCallback = new NiceMock<MockCallback>(true);
    CallbackSmartPtr callback(mockCallback);
    MpageList dirtyList;

    ON_CALL(journal, IsEnabled).WillByDefault(Return(true));
    ON_CALL(vsaMap, GetDirtyVsaMapPages).WillByDefault(Return(dirtyList));

    EXPECT_CALL(journalWriter, AddBlockMapUpdatedLog(volumeIo, dirtyList, _)).WillOnce(Return(-1));

    int actual = metaUpdater.UpdateBlockMap(volumeIo, callback);
    EXPECT_TRUE(actual < 0);
}

// TODO (huijeong.kim) : Inject callback event for unit test
TEST(BlockMapUpdate, DISABLED_UpdateBlockMap_testIfExecutedSuccessfullyWhenJournalDisabled)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler);

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    NiceMock<MockCallback>* mockCallback = new NiceMock<MockCallback>(true);
    CallbackSmartPtr callback(mockCallback);

    ON_CALL(journal, IsEnabled).WillByDefault(Return(false));

    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    int actual = metaUpdater.UpdateBlockMap(volumeIo, callback);
    int expected = 0;

    EXPECT_EQ(actual, expected);
}

} // namespace pos
