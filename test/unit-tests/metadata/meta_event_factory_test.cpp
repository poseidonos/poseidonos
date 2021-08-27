#include "src/metadata/meta_event_factory.h"

#include <gtest/gtest.h>

#include "src/mapper_service/mapper_service.h"
#include "src/metadata/block_map_update.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MetaEventFactory, MetaEventFactory_testIfConstructedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;

    MetaEventFactory factory(&vsaMap, &blockAllocator, &wbStripeAllocator);
}

TEST(MetaEventFactory, CreateBlockMapUpdateEvent_testIfBlockMapUpdateEventCreatedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIBlockAllocator> blockAllocator;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;

    int arrayId = 2;
    MapperServiceSingleton::Instance()->RegisterMapper("", arrayId, &vsaMap, nullptr, nullptr, nullptr, nullptr);

    MetaEventFactory factory(&vsaMap, &blockAllocator, &wbStripeAllocator);

    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    ON_CALL(*mockVolumeIo, GetArrayId).WillByDefault(Return(arrayId));

    CallbackSmartPtr actual = factory.CreateBlockMapUpdateEvent(mockVolumeIoPtr);

    EXPECT_EQ(typeid(*actual.get()), typeid(BlockMapUpdate));
}

} // namespace pos
