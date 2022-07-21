#include "src/gc/copier_read_completion.h"

#include <gtest/gtest.h>
#include <test/unit-tests/allocator/i_block_allocator_mock.h>
#include <test/unit-tests/allocator/i_context_manager_mock.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/copier_meta_mock.h>
#include <test/unit-tests/gc/gc_status_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/gc/reverse_map_load_completion_mock.h>
#include <test/unit-tests/gc/stripe_copy_submission_mock.h>
#include <test/unit-tests/gc/victim_stripe_mock.h>
#include <test/unit-tests/lib/bitmap_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/mapper/i_reversemap_mock.h>
#include <test/unit-tests/mapper/reversemap/reverse_map_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

#include <test/unit-tests/gc/gc_flush_submission_mock.h>
#include <test/unit-tests/volume/i_volume_io_manager_mock.h>
#include <test/unit-tests/event_scheduler/event_scheduler_mock.h>

#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/resource_manager/memory_manager_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
using ::testing::AtLeast;

namespace pos
{
static const uint32_t GC_BUFFER_COUNT = 512;
static const uint32_t GC_CONCURRENT_COUNT = 16;
static const uint32_t GC_VICTIM_SEGMENT_COUNT = 2;
class CopierReadCompletionTestFixture : public ::testing::Test
{
public:
    CopierReadCompletionTestFixture(void)
    : copierReadCompletion(nullptr),
      gcStatus(nullptr),
      array(nullptr),
      meta(nullptr),
      inUseBitmap(nullptr),
      gcStripeManager(nullptr),
      iReverseMap(nullptr),
      volumeEventPublisher(nullptr),
      victimStripe(nullptr),
      victimStripes(nullptr),
      gcBufferPool(nullptr),
      inputVolumeManager(nullptr),
      inputEventScheduler(nullptr)
    {
    }

    virtual ~CopierReadCompletionTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        testListIndex = 0;
        testStripeId = 100;
        testVolumeId = 1;

        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        gcStatus = new NiceMock<MockGcStatus>;
        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        memoryManager = new MockMemoryManager();
        EXPECT_CALL(*memoryManager, CreateBufferPool).WillRepeatedly(Return(nullptr));
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, volumeEventPublisher, memoryManager);
        iReverseMap = new NiceMock<MockIReverseMap>();
        victimStripe = new NiceMock<MockVictimStripe>(array, iReverseMap, nullptr, nullptr, nullptr);
        victimStripes = new std::vector<std::vector<VictimStripe*>>;
        victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
        for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
        {
            for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
            {
                (*victimStripes)[stripeIndex].push_back(new NiceMock<MockVictimStripe>(array, iReverseMap, nullptr, nullptr, nullptr));
            }
        }

        gcBufferPool = new std::vector<BufferPool*>;
        for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
        {
            BufferInfo info;
            gcBufferPool->push_back(new NiceMock<MockBufferPool>(info, 0, nullptr));
        }

        meta = new NiceMock<MockCopierMeta>(array, udSize, inUseBitmap, gcStripeManager, victimStripes, gcBufferPool);

        inputFlushEvent = std::make_shared<MockGcFlushSubmission>(
                array->GetName(), blkInfoList, testVolumeId, dataBuffer, gcStripeManager,
                nullptr, nullptr, nullptr, nullptr, nullptr);
        inputVolumeManager = new NiceMock<MockIVolumeIoManager>();
        inputEventScheduler = new NiceMock<MockEventScheduler>;

        dataBuffer = new GcWriteBuffer();
        int error = posix_memalign(&buffer, ALLOCATION_SIZE_BYTE, ALLOCATION_SIZE_BYTE * 1);
        assert(0 == error);
        dataBuffer->push_back(buffer);
    }

    virtual void
    TearDown(void)
    {
        delete copierReadCompletion;
        delete array;
        delete meta;
        delete inUseBitmap;
        delete volumeEventPublisher;
        delete gcStatus;
        delete victimStripe;
        delete inputVolumeManager;
        delete inputEventScheduler;
        delete memoryManager;

        inputFlushEvent = nullptr;
        mockBlkInfoList.clear();

        std::free(buffer);
        dataBuffer->clear();
    }

protected:
    CopierReadCompletion* copierReadCompletion;

    uint32_t testListIndex;
    StripeId testStripeId;
    uint32_t testVolumeId;

    NiceMock<MockGcStatus>* gcStatus;
    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockCopierMeta>* meta;
    NiceMock<MockBitMapMutex>* inUseBitmap;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockIReverseMap>* iReverseMap;
    NiceMock<MockVictimStripe>* victimStripe;
    NiceMock<MockIVolumeIoManager>* inputVolumeManager;
    NiceMock<MockEventScheduler>* inputEventScheduler;

    std::vector<std::vector<VictimStripe*>>* victimStripes;
    std::vector<BufferPool*>* gcBufferPool;

    void* buffer;
    std::vector<BlkInfo>* blkInfoList;
    GcWriteBuffer* dataBuffer;
    MockMemoryManager* memoryManager;
    EventSmartPtr inputFlushEvent;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;
    static const uint32_t ALLOCATION_SIZE_BYTE = 2 * 1024 * 1024;
    std::list<BlkInfo> mockBlkInfoList;

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

TEST_F(CopierReadCompletionTestFixture, CopierReadCompletion_)
{
}

TEST_F(CopierReadCompletionTestFixture, Execute_testIfGcStripemanagerFailedtoAllocateBlockWhenCopierReadCompletionExecutesProperly)
{
    // given Read block info
    for (uint64_t offset = 0; offset < partitionLogicalSize.blksPerChunk; offset++)
    {
        BlkAddr rba = offset;
        uint32_t volId = testVolumeId;
        VirtualBlkAddr vsa = {.stripeId = testStripeId, .offset = offset};
        BlkInfo blkInfo = {.rba = rba, .volID = volId, .vsa = vsa};
        mockBlkInfoList.push_back(blkInfo);
    }

    EXPECT_CALL(*victimStripe, GetBlkInfoList(testListIndex)).WillRepeatedly(ReturnRef(mockBlkInfoList));
    copierReadCompletion = new CopierReadCompletion(victimStripe, testListIndex, buffer,
                            meta, testStripeId,
                            inputFlushEvent, inputVolumeManager, inputEventScheduler);
    // given did not allocate block from gc stripe
    EXPECT_CALL(*meta, GetGcStripeManager).WillRepeatedly(Return(gcStripeManager));
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = 0};
    EXPECT_CALL(*gcStripeManager, AllocateWriteBufferBlks(testVolumeId, _)).WillOnce(Return(expectGcAllocateBlks));

    // when event execute
    // then return false
    EXPECT_TRUE(copierReadCompletion->Execute() == false);
}


TEST_F(CopierReadCompletionTestFixture, Execute_testIfGcStripeManaerDecreaseRemainingAndCheckIsFullWhenCopierReadCompletionExecutesProperly)
{
    // given read block info
    for (uint64_t offset = 0; offset < partitionLogicalSize.blksPerChunk; offset++)
    {
        BlkAddr rba = offset;
        uint32_t volId = testVolumeId;
        VirtualBlkAddr vsa = {.stripeId = testStripeId, .offset = offset};
        BlkInfo blkInfo = {.rba = rba, .volID = volId, .vsa = vsa};
        mockBlkInfoList.push_back(blkInfo);

        EXPECT_CALL(*gcStripeManager, SetBlkInfo(testVolumeId, offset, _)).Times(1);
    }

    EXPECT_CALL(*victimStripe, GetBlkInfoList(testListIndex)).WillRepeatedly(ReturnRef(mockBlkInfoList));
    copierReadCompletion = new CopierReadCompletion(victimStripe, testListIndex, buffer,
                            meta, testStripeId,
                            inputFlushEvent, inputVolumeManager, inputEventScheduler);

    EXPECT_CALL(*meta, GetGcStripeManager).WillRepeatedly(Return(gcStripeManager));
    // given gcStripe allocate all blocks
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = partitionLogicalSize.blksPerChunk};
    EXPECT_CALL(*gcStripeManager, AllocateWriteBufferBlks(testVolumeId, _)).WillOnce(Return(expectGcAllocateBlks));
    EXPECT_CALL(*gcStripeManager, GetWriteBuffer(testVolumeId)).WillRepeatedly(Return(dataBuffer));
    EXPECT_CALL(*gcStripeManager, GetBlkInfoList(testVolumeId)).WillRepeatedly(Return(blkInfoList));

    // given gc stripe is not full
    EXPECT_CALL(*gcStripeManager, DecreaseRemainingAndCheckIsFull(testVolumeId, _)).WillOnce(Return(false));

    EXPECT_CALL(*inputVolumeManager, DecreasePendingIOCount(testVolumeId, VolumeIoType::InternalIo, partitionLogicalSize.blksPerChunk)).Times(1);
    EXPECT_CALL(*meta, ReturnBuffer(testStripeId, buffer)).Times(1);
    EXPECT_CALL(*meta, SetDoneCopyBlks(partitionLogicalSize.blksPerChunk)).Times(1);

    // when Execute
    // then return true
    EXPECT_TRUE(copierReadCompletion->Execute() == true);
}

TEST_F(CopierReadCompletionTestFixture, Execute_testIfGcStripeManaerAllocatesWriteBufferBlksAndDecreaseRemainingAndCheckIsFullWhenCopierReadCompletionExecutesProperly)
{
    // given read block info
    for (uint64_t offset = 0; offset < partitionLogicalSize.blksPerChunk; offset++)
    {
        BlkAddr rba = offset;
        uint32_t volId = testVolumeId;
        VirtualBlkAddr vsa = {.stripeId = testStripeId, .offset = offset};
        BlkInfo blkInfo = {.rba = rba, .volID = volId, .vsa = vsa};
        mockBlkInfoList.push_back(blkInfo);

        EXPECT_CALL(*gcStripeManager, SetBlkInfo(testVolumeId, offset, _)).Times(1);
    }

    EXPECT_CALL(*victimStripe, GetBlkInfoList(testListIndex)).WillRepeatedly(ReturnRef(mockBlkInfoList));
    copierReadCompletion = new CopierReadCompletion(victimStripe, testListIndex, buffer,
                            meta, testStripeId,
                            inputFlushEvent, inputVolumeManager, inputEventScheduler);

    EXPECT_CALL(*meta, GetGcStripeManager).WillRepeatedly(Return(gcStripeManager));
    // given gcStripe allocate all blocks
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = partitionLogicalSize.blksPerChunk};
    EXPECT_CALL(*gcStripeManager, AllocateWriteBufferBlks(testVolumeId, _)).WillOnce(Return(expectGcAllocateBlks));
    EXPECT_CALL(*gcStripeManager, GetWriteBuffer(testVolumeId)).WillRepeatedly(Return(dataBuffer));
    EXPECT_CALL(*gcStripeManager, GetBlkInfoList(testVolumeId)).WillRepeatedly(Return(blkInfoList));

    // given using gc stripe is full
    EXPECT_CALL(*gcStripeManager, DecreaseRemainingAndCheckIsFull(testVolumeId, _)).WillOnce(Return(true));

    // given return success when volumemanager increase pending io
    EXPECT_CALL(*inputVolumeManager, IncreasePendingIOCountIfNotZero(testVolumeId, VolumeIoType::InternalIo, 1)).WillOnce(Return(0));
    EXPECT_CALL(*inputEventScheduler, EnqueueEvent(inputFlushEvent)).Times(1);
    EXPECT_CALL(*gcStripeManager, SetFlushed(testVolumeId)).Times(1);

    EXPECT_CALL(*inputVolumeManager, DecreasePendingIOCount(testVolumeId, VolumeIoType::InternalIo, partitionLogicalSize.blksPerChunk)).Times(1);
    EXPECT_CALL(*meta, ReturnBuffer(testStripeId, buffer)).Times(1);
    EXPECT_CALL(*meta, SetDoneCopyBlks(partitionLogicalSize.blksPerChunk)).Times(1);

    // when Execute
    // then return true
    EXPECT_TRUE(copierReadCompletion->Execute() == true);
}

TEST_F(CopierReadCompletionTestFixture, Execute_testIfFaliedThatVolumeManagerIncreasePendingIoWhenCopierReadCompletionExecutes)
{
    // given read block info
    for (uint64_t offset = 0; offset < partitionLogicalSize.blksPerChunk; offset++)
    {
        BlkAddr rba = offset;
        uint32_t volId = testVolumeId;
        VirtualBlkAddr vsa = {.stripeId = testStripeId, .offset = offset};
        BlkInfo blkInfo = {.rba = rba, .volID = volId, .vsa = vsa};
        mockBlkInfoList.push_back(blkInfo);

        EXPECT_CALL(*gcStripeManager, SetBlkInfo(testVolumeId, offset, _)).Times(1);
    }

    EXPECT_CALL(*victimStripe, GetBlkInfoList(testListIndex)).WillRepeatedly(ReturnRef(mockBlkInfoList));
    copierReadCompletion = new CopierReadCompletion(victimStripe, testListIndex, buffer,
                            meta, testStripeId,
                            inputFlushEvent, inputVolumeManager, inputEventScheduler);

    EXPECT_CALL(*meta, GetGcStripeManager).WillRepeatedly(Return(gcStripeManager));
    // given gcStripe allocate all blocks
    GcAllocateBlks expectGcAllocateBlks = {.startOffset = 0, .numBlks = partitionLogicalSize.blksPerChunk};
    EXPECT_CALL(*gcStripeManager, AllocateWriteBufferBlks(testVolumeId, _)).WillOnce(Return(expectGcAllocateBlks));
    EXPECT_CALL(*gcStripeManager, GetWriteBuffer(testVolumeId)).WillRepeatedly(Return(dataBuffer));

    blkInfoList = new std::vector<BlkInfo>;
    EXPECT_CALL(*gcStripeManager, GetBlkInfoList(testVolumeId)).WillRepeatedly(Return(blkInfoList));
    // given using gc stripe is full
    EXPECT_CALL(*gcStripeManager, DecreaseRemainingAndCheckIsFull(testVolumeId, _)).WillOnce(Return(true));
    // given return fail when volumemanager increase pending io
    EXPECT_CALL(*inputVolumeManager, IncreasePendingIOCountIfNotZero(testVolumeId, VolumeIoType::InternalIo, 1)).WillOnce(Return(2010));
    EXPECT_CALL(*gcStripeManager, SetFlushed(testVolumeId)).Times(1);
    EXPECT_CALL(*gcStripeManager, ReturnBuffer(dataBuffer)).Times(1);
    EXPECT_CALL(*gcStripeManager, SetFinished).Times(1);

    EXPECT_CALL(*inputVolumeManager, DecreasePendingIOCount(testVolumeId, VolumeIoType::InternalIo, partitionLogicalSize.blksPerChunk)).Times(1);
    EXPECT_CALL(*meta, ReturnBuffer(testStripeId, buffer)).Times(1);
    EXPECT_CALL(*meta, SetDoneCopyBlks(partitionLogicalSize.blksPerChunk)).Times(1);

    // when execute
    // then return true
    EXPECT_TRUE(copierReadCompletion->Execute() == true);
}
} // namespace pos
