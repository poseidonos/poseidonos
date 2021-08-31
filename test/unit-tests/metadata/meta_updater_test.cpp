#include "src/metadata/meta_updater.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/journal_service/i_journal_manager_mock.h"
#include "test/unit-tests/journal_service/i_journal_writer_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"
#include "test/unit-tests/metadata/meta_event_factory_mock.h"
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
TEST(MetaUpdater, MetaUpdater_testIfProductConstructorExecutedSuccessfully)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;

    // When
    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler);

    // Then : Do nothing
}

TEST(MetaUpdater, MetaUpdater_testIfUTConstructorExecutedSuccessfully)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockMetaEventFactory>* metaEventFactory = new NiceMock<MockMetaEventFactory>;

    // When
    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory);

    // Then : Do nothing
}
TEST(MetaUpdater, UpdateBlockMap_testIfExecutedSuccessullfyWhenJournalEnabled)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockMetaEventFactory>* metaEventFactory = new NiceMock<MockMetaEventFactory>;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory);

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr clientCallback(new NiceMock<MockCallback>(true));
    CallbackSmartPtr metaCallback(new NiceMock<MockCallback>(true));
    MpageList dirtyList;

    ON_CALL(*metaEventFactory, CreateBlockMapUpdateEvent).WillByDefault(Return(metaCallback));
    ON_CALL(journal, IsEnabled).WillByDefault(Return(true));
    ON_CALL(vsaMap, GetDirtyVsaMapPages).WillByDefault(Return(dirtyList));

    EXPECT_CALL(journalWriter, AddBlockMapUpdatedLog(volumeIo, dirtyList, _)).WillOnce(Return(0));

    int expected = 0;
    int actual = metaUpdater.UpdateBlockMap(volumeIo, clientCallback);

    EXPECT_EQ(actual, expected);
}

TEST(MetaUpdater, UpdateBlockMap_testIfFailsWhenJournalWriteFails)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockMetaEventFactory>* metaEventFactory = new NiceMock<MockMetaEventFactory>;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory);

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr clientCallback(new NiceMock<MockCallback>(true));
    CallbackSmartPtr metaCallback(new NiceMock<MockCallback>(true));
    MpageList dirtyList;

    ON_CALL(*metaEventFactory, CreateBlockMapUpdateEvent).WillByDefault(Return(metaCallback));
    ON_CALL(journal, IsEnabled).WillByDefault(Return(true));
    ON_CALL(vsaMap, GetDirtyVsaMapPages).WillByDefault(Return(dirtyList));

    EXPECT_CALL(journalWriter, AddBlockMapUpdatedLog(volumeIo, dirtyList, _)).WillOnce(Return(-1));

    int actual = metaUpdater.UpdateBlockMap(volumeIo, clientCallback);
    EXPECT_TRUE(actual < 0);
}

TEST(MetaUpdater, UpdateBlockMap_testIfExecutedSuccessfullyWhenJournalDisabled)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockMetaEventFactory>* metaEventFactory = new NiceMock<MockMetaEventFactory>;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory);

    NiceMock<MockVolumeIo>* mockVolumeIo = new NiceMock<MockVolumeIo>(nullptr, 0, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr clientCallback(new NiceMock<MockCallback>(true));
    CallbackSmartPtr metaCallback(new NiceMock<MockCallback>(true));

    ON_CALL(*metaEventFactory, CreateBlockMapUpdateEvent).WillByDefault(Return(metaCallback));
    ON_CALL(journal, IsEnabled).WillByDefault(Return(false));

    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    int actual = metaUpdater.UpdateBlockMap(volumeIo, clientCallback);
    int expected = 0;

    EXPECT_EQ(actual, expected);
}

} // namespace pos
