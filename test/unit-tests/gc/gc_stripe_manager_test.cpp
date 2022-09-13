#include "src/gc/gc_stripe_manager.h"

#include <gtest/gtest.h>
#include <src/include/address_type.h>
#include "src/include/pos_event_id.h"
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

#include "test/unit-tests/resource_manager/memory_manager_mock.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;
namespace pos
{
class GcStripeManagerTestFixture : public ::testing::Test
{
public:
    GcStripeManagerTestFixture(void)
    : gcStripeManager(nullptr),
      array(nullptr),
      volumeEventPublisher(nullptr),
      memoryManager(nullptr)
    {
    }

    virtual ~GcStripeManagerTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));
        EXPECT_CALL(*array, GetName()).WillRepeatedly(Return("POSArray"));
        EXPECT_CALL(*array, GetIndex()).WillRepeatedly(Return(0));

        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        BufferInfo info;
        gcWriteBufferPool = new NiceMock<MockBufferPool>(info, 0, nullptr);
        memoryManager = new NiceMock<MockMemoryManager>();
        EXPECT_CALL(*memoryManager, CreateBufferPool)
            .WillRepeatedly(Return(gcWriteBufferPool));
        EXPECT_CALL(*memoryManager, DeleteBufferPool).WillRepeatedly(
            [](BufferPool* pool) -> bool {
                delete pool;
                return true;
            });
        gcStripeManager =
            new GcStripeManager(array, volumeEventPublisher, memoryManager);
    }

    virtual void
    TearDown(void)
    {
        delete array;
        delete gcStripeManager;
        delete memoryManager;
    }

protected:
    GcStripeManager* gcStripeManager;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockBufferPool>* gcWriteBufferPool;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockMemoryManager>* memoryManager;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 4,
    .blksPerStripe = 16,
    .chunksPerStripe = 4,
    .stripesPerSegment = 32,
    .totalStripes = 3200,
    .totalSegments = 100,
    };
};

TEST_F(GcStripeManagerTestFixture, AllocateBlocks_testAllocateGcWriteBufferBlksWithWriteBuffer)
{
    uint32_t volId = 0;
    uint32_t numBlks = partitionLogicalSize.blksPerStripe;
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = numBlks};
    GcAllocateBlks testGcAllocateBlks;

    // given not enough gc write buffer
    EXPECT_CALL(*gcWriteBufferPool, TryGetBuffer).WillOnce(Return((void*)0x20000000)).WillOnce(Return(nullptr));
    // when allocate write buffer blks
    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks);
    // then return allocate 0 blk
    EXPECT_TRUE(testGcAllocateBlks.numBlks == 0);

    // given enough gc write buffer
    EXPECT_CALL(*gcWriteBufferPool, TryGetBuffer).WillRepeatedly(Return((void*)0x20000000));
    // when allocate write buffer blks
    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks);
    // then return request blks cnt
    EXPECT_TRUE(testGcAllocateBlks.numBlks == numBlks);
}

TEST_F(GcStripeManagerTestFixture, AllocateBlocks_testAllocateGcWriteBufferBlksBaseOnActiveGcStripe)
{
    uint32_t volId = 0;
    uint32_t numBlks = partitionLogicalSize.blksPerStripe;
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = numBlks / 2};
    GcAllocateBlks testGcAllocateBlks;

    // given enough write buffer and request to allocate blks per stripe
    EXPECT_CALL(*gcWriteBufferPool, TryGetBuffer).WillRepeatedly(Return((void*)0x20000000));
    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks/2);
    EXPECT_TRUE(testGcAllocateBlks.numBlks == numBlks / 2);
    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks);
    EXPECT_TRUE(testGcAllocateBlks.numBlks == numBlks / 2);

    // when allocate write buffer blks, active gc stripe already full
    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks);
    // then allocate blks cnt is 0
    EXPECT_TRUE(testGcAllocateBlks.numBlks == 0);
}

TEST_F(GcStripeManagerTestFixture, AllocateBlocks_testIf)
{
    uint32_t volId = 0;
    uint32_t numBlks = partitionLogicalSize.blksPerStripe;
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = numBlks};
    GcAllocateBlks testGcAllocateBlks;

    // given write buffer and allocate write buffer blks
    EXPECT_CALL(*gcWriteBufferPool, TryGetBuffer).WillRepeatedly(Return((void*)0x20000000));
    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks);
    EXPECT_TRUE(testGcAllocateBlks.numBlks == numBlks);
    // when get write buffer
    // then return GcWriteBuffer
    GcWriteBuffer* writeBuffer = gcStripeManager->GetWriteBuffer(volId);
    for (auto buffer : *writeBuffer)
    {
        EXPECT_TRUE(buffer == (void*)0x20000000);
    }

    // when set blk info
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        VirtualBlkAddr vsa = {.stripeId = 100, .offset = offset};
        BlkInfo blkInfo = {.rba = 0, .volID = volId, .vsa = vsa};
        gcStripeManager->SetBlkInfo(volId, offset, blkInfo);
    }
    // when get blk info
    std::vector<BlkInfo>* blkInfoList = gcStripeManager->GetBlkInfoList(volId);
    std::vector<BlkInfo>::iterator blkInfo = blkInfoList->begin();
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        // then same blk info
        VirtualBlkAddr vsa = {.stripeId = 100, .offset = offset};
        EXPECT_TRUE(vsa == blkInfo->vsa);
        blkInfo++;
    }

    // when decrease remainig count and check remaining count at active gc stripe
    bool isFull = gcStripeManager->DecreaseRemainingAndCheckIsFull(volId, expectGcAllocateBlks.numBlks);
    // then is full return true
    EXPECT_TRUE(isFull == true);

    // when set flushed
    gcStripeManager->SetFlushed(volId);
    // then isallfinished return false
    EXPECT_TRUE(gcStripeManager->IsAllFinished() == false);

    // when set finished
    gcStripeManager->SetFinished();
    gcStripeManager->ReturnBuffer(writeBuffer);
    // then return all finished
    EXPECT_TRUE(gcStripeManager->IsAllFinished() == true);
}

TEST_F(GcStripeManagerTestFixture, VolumeEvent_testVolumeDeleteWhenSetBlkInfo)
{
    uint32_t volId = 0;
    uint32_t numBlks = partitionLogicalSize.blksPerStripe;
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = numBlks};
    GcAllocateBlks testGcAllocateBlks;

    EXPECT_CALL(*gcWriteBufferPool, TryGetBuffer).WillRepeatedly(Return((void*)0x20000000));

    testGcAllocateBlks = gcStripeManager->AllocateWriteBufferBlks(volId, numBlks);
    // when get write buffer
    // then return GcWriteBuffer
    GcWriteBuffer* writeBuffer = gcStripeManager->GetWriteBuffer(volId);
    for (auto buffer : *writeBuffer)
    {
        EXPECT_TRUE(buffer == (void*)0x20000000);
    }
    EXPECT_TRUE(testGcAllocateBlks.numBlks == numBlks);
    // when set blk info
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        VirtualBlkAddr vsa = {.stripeId = 100, .offset = offset};
        BlkInfo blkInfo = {.rba = 0, .volID = volId, .vsa = vsa};
        gcStripeManager->SetBlkInfo(volId, offset, blkInfo);
    }
    // when get blk info
    std::vector<BlkInfo>* blkInfoList = gcStripeManager->GetBlkInfoList(volId);
    std::vector<BlkInfo>::iterator blkInfo = blkInfoList->begin();
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        // then same blk info
        VirtualBlkAddr vsa = {.stripeId = 100, .offset = offset};
        EXPECT_TRUE(vsa == blkInfo->vsa);
        blkInfo++;
    }

    // when volume delete
    VolumeEventBase volumeEventBase;
    volumeEventBase.volId = volId;
    VolumeEventPerf volumeMountPerf;
    VolumeArrayInfo volumeArrayInfo;

    int expected = EID(VOL_EVENT_OK);
    EXPECT_TRUE(gcStripeManager->VolumeDeleted(&volumeEventBase, &volumeArrayInfo) == expected);

    // then delete blk Info list and write buffer
    blkInfoList = gcStripeManager->GetBlkInfoList(volId);
    EXPECT_TRUE(blkInfoList == nullptr);
    writeBuffer = gcStripeManager->GetWriteBuffer(volId);
    EXPECT_TRUE(writeBuffer == nullptr);
}

TEST_F(GcStripeManagerTestFixture, VolumeEvent_Invoke)
{
    VolumeEventBase volumeEventBase;
    volumeEventBase.volId = 1;
    VolumeEventPerf volumeMountPerf;
    VolumeArrayInfo volumeArrayInfo;

    int expected = EID(VOL_EVENT_OK);

    EXPECT_TRUE(gcStripeManager->VolumeCreated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo) == expected); // trival volume create
    EXPECT_TRUE(gcStripeManager->VolumeMounted(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo) == expected); // trival volume mounted
    EXPECT_TRUE(gcStripeManager->VolumeUnmounted(&volumeEventBase, &volumeArrayInfo) == expected); // trival volume unmounted
    EXPECT_TRUE(gcStripeManager->VolumeLoaded(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo) == expected); // trival volume loaded
    EXPECT_TRUE(gcStripeManager->VolumeUpdated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo) == expected); // trival volume updated
    std::vector<int> volList;
    gcStripeManager->VolumeDetached(volList, &volumeArrayInfo); // trival volume detached
}

} // namespace pos
