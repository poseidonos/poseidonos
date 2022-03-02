#include "src/metadata/meta_event_factory.h"

#include <gtest/gtest.h>

#include "src/mapper_service/mapper_service.h"
#include "src/metadata/block_map_update.h"
#include "src/metadata/gc_map_update.h"
#include "src/metadata/stripe_map_update.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace pos
{
TEST(MetaEventFactory, MetaEventFactory_testIfConstructedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIArrayInfo> arrayInfo;
    MetaEventFactory factory(&vsaMap, &stripeMap, &segmentCtx, &wbStripeAllocator, &contextManager, &arrayInfo);
}

TEST(MetaEventFactory, CreateBlockMapUpdateEvent_testIfBlockMapUpdateEventCreatedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIArrayInfo> arrayInfo;

    int arrayId = 2;
    MapperServiceSingleton::Instance()->RegisterMapper("", arrayId, &vsaMap, nullptr, nullptr, nullptr, nullptr);

    MetaEventFactory factory(&vsaMap, &stripeMap, &segmentCtx, &wbStripeAllocator, &contextManager, &arrayInfo);

    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    ON_CALL(*mockVolumeIo, GetArrayId).WillByDefault(Return(arrayId));

    CallbackSmartPtr actual = factory.CreateBlockMapUpdateEvent(mockVolumeIoPtr);

    EXPECT_EQ(typeid(*actual.get()), typeid(BlockMapUpdate));
}

TEST(MetaEventFactory, CreateStripeMapUpdateEvent_testIfStripeMapUpdateEventIsCreatedSuccessfully)
{
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;

    MetaEventFactory factory(nullptr, &stripeMap, nullptr, nullptr, &contextManager, nullptr);

    NiceMock<MockStripe> stripe;
    CallbackSmartPtr actual = factory.CreateStripeMapUpdateEvent(&stripe);

    EXPECT_EQ(typeid(*actual.get()), typeid(StripeMapUpdate));
}

TEST(MetaEventFactory, CreateGcMapUpdateEvent_testIfGcMapUpdateEventCreatedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIArrayInfo> arrayInfo;

    int arrayId = 2;
    MapperServiceSingleton::Instance()->RegisterMapper("", arrayId, &vsaMap, nullptr, nullptr, nullptr, nullptr);

    MetaEventFactory factory(&vsaMap, &stripeMap, &segmentCtx, &wbStripeAllocator, &contextManager, &arrayInfo);

    NiceMock<MockStripe> stripe;
    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t > invalidSegCnt;
    PartitionLogicalSize partitionLogicalSize;
    partitionLogicalSize.stripesPerSegment = 1;
    EXPECT_CALL(arrayInfo, GetSizeInfo).WillRepeatedly(Return(&partitionLogicalSize));

    CallbackSmartPtr actual = factory.CreateGcMapUpdateEvent(&stripe, mapUpdateInfoList, invalidSegCnt);

    EXPECT_EQ(typeid(*actual.get()), typeid(GcMapUpdate));
}
} // namespace pos
