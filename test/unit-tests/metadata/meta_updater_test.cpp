/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/metadata/meta_updater.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/journal_manager/i_journal_manager_mock.h"
#include "test/unit-tests/journal_manager/i_journal_writer_mock.h"
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
    NiceMock<MockIArrayInfo> arrayInfo;

    // When
    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, &arrayInfo);

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
    NiceMock<MockIArrayInfo> arrayInfo;

    // When
    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory, &arrayInfo);

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
    NiceMock<MockIArrayInfo> arrayInfo;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory, &arrayInfo);

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
    NiceMock<MockIArrayInfo> arrayInfo;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory, &arrayInfo);

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
    NiceMock<MockIArrayInfo> arrayInfo;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory, &arrayInfo);

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

TEST(MetaUpdater, UpdateGcMap_testIfExecutedSuccessullfyWhenJournalEnabled)
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
    NiceMock<MockIArrayInfo> arrayInfo;

    MetaUpdater metaUpdater(&vsaMap, &stripeMap, &contextManager, &blockAllocator,
        &wbStripeAllocator, &journal, &journalWriter, &eventScheduler, metaEventFactory, &arrayInfo);

    // When
    uint32_t stripeId = 100;
    NiceMock<MockStripe> stripe;
    GcStripeMapUpdateList mapUpdateInfoList;
    mapUpdateInfoList.volumeId = 1;
    uint32_t blockCount = 1;
    std::map<SegmentId, uint32_t> invalidSegCnt;

    CallbackSmartPtr clientCallback(new NiceMock<MockCallback>(true));
    CallbackSmartPtr metaCallback(new NiceMock<MockCallback>(true));

    ON_CALL(*metaEventFactory, CreateGcMapUpdateEvent).WillByDefault(Return(metaCallback));
    ON_CALL(journal, IsEnabled).WillByDefault(Return(true));
    ON_CALL(stripe, GetVsid).WillByDefault(Return(stripeId));

    PartitionLogicalSize partitionLogicalSize;
    partitionLogicalSize.blksPerStripe = 10;
    ON_CALL(arrayInfo, GetSizeInfo).WillByDefault(Return(&partitionLogicalSize));

    MapPageList dirtyMap;
    for (uint32_t offset = 0; offset < partitionLogicalSize.blksPerStripe; offset++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {offset, mapUpdateInfoList.volumeId};
        EXPECT_CALL(stripe, GetReverseMapEntry(offset)).WillRepeatedly(Return(revMapEntry));

        MpageList dirty;
        dirty.insert(offset);
        EXPECT_CALL(vsaMap, GetDirtyVsaMapPages(mapUpdateInfoList.volumeId, offset, blockCount)).WillOnce(Return(dirty));
        dirtyMap[mapUpdateInfoList.volumeId].insert(dirty.begin(), dirty.end());
    }
    MpageList dirty;
    dirty.insert(10);
    EXPECT_CALL(stripeMap, GetDirtyStripeMapPages(stripeId)).WillOnce(Return(dirty));
    dirtyMap[STRIPE_MAP_ID] = dirty;

    // Then
    int expected = 0;
    int actual = metaUpdater.UpdateGcMap(&stripe, mapUpdateInfoList, invalidSegCnt, clientCallback);

    EXPECT_EQ(actual, expected);
}


TEST(MetaUpdater, UpdateGcMap_testIfExecutedSuccessullfyWhenJournalDisable)
{
    // Given
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockMetaEventFactory>* metaEventFactory = new NiceMock<MockMetaEventFactory>;

    MetaUpdater metaUpdater(nullptr, nullptr, nullptr, nullptr, nullptr, &journal, nullptr, &eventScheduler, metaEventFactory, nullptr);

    // When
    uint32_t stripeId = 100;
    NiceMock<MockStripe> stripe;
    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t> invalidSegCnt;
    CallbackSmartPtr metaCallback(new NiceMock<MockCallback>(true));

    ON_CALL(*metaEventFactory, CreateGcMapUpdateEvent).WillByDefault(Return(metaCallback));
    ON_CALL(journal, IsEnabled).WillByDefault(Return(false));
    ON_CALL(stripe, GetVsid).WillByDefault(Return(stripeId));
    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    // Then
    int expected = 0;
    int actual = metaUpdater.UpdateGcMap(&stripe, mapUpdateInfoList, invalidSegCnt, nullptr);

    EXPECT_EQ(actual, expected);
}
} // namespace pos
