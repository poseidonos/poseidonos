#include "src/array/partition/stripe_partition.h"

#include <gtest/gtest.h>

#include "src/array/device/array_device.h"
#include "src/array/ft/raid1.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/bio/ubio.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/array/ft/raid1_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"

using ::testing::Return;

namespace pos
{
extern const char* PARTITION_TYPE_STR;
TEST(StripePartition, StripePartition_testIfConstructorSetsLogicalSizeProperly)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .blksPerChunk = 10,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    MockRaid1* mockRaid1 = new MockRaid1(&physicalSize);
    vector<ArrayDevice*> devs;

    // When
    StripePartition sPartition("mock-array", PartitionType::USER_DATA, physicalSize, devs, mockRaid1);

    // Then
    const PartitionLogicalSize* pLogicalSize = sPartition.GetLogicalSize();
    ASSERT_EQ(physicalSize.blksPerChunk, pLogicalSize->blksPerChunk);
    ASSERT_EQ(physicalSize.stripesPerSegment, pLogicalSize->stripesPerSegment);
    ASSERT_EQ(physicalSize.totalSegments, pLogicalSize->totalSegments);
    ASSERT_EQ(physicalSize.stripesPerSegment * physicalSize.totalSegments, pLogicalSize->totalStripes);
}

TEST(StripePartition, Translate_testIfFtBlkAddrIsMappedToPhysicalBlkAddr)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .startLba = 5,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    StripeId MAX_BLK_OFFSET = physicalSize.blksPerChunk * physicalSize.chunksPerStripe - 1;
    StripeId TOTAL_STRIPES = physicalSize.stripesPerSegment * physicalSize.totalSegments;
    MockRaid1* mockRaid1 = new MockRaid1(&physicalSize);
    vector<ArrayDevice*> devs;
    for (int i = 0; i < physicalSize.chunksPerStripe; i += 1)
    {
        devs.push_back(new ArrayDevice(nullptr));
    }
    LogicalBlkAddr src{
        .stripeId = TOTAL_STRIPES - 1,
        .offset = MAX_BLK_OFFSET / 2};
    PhysicalBlkAddr dest;
    StripePartition sPartition("mock-array", PartitionType::USER_DATA, physicalSize, devs, mockRaid1);

    EXPECT_CALL(*mockRaid1, Translate).WillOnce([](FtBlkAddr& fba, const LogicalBlkAddr& src)
    {
        // the actual logic of Translate() is not of interest, so I'm just passing in src's stripe id and offset
        fba.stripeId = src.stripeId;
        fba.offset = src.offset;
        return 0;
    });

    // When
    int actual = sPartition.Translate(dest, src);

    // Then
    ASSERT_EQ(0, actual);
    int expectedChunkIndex = src.offset / physicalSize.blksPerChunk;
    ASSERT_EQ(devs.at(expectedChunkIndex), dest.arrayDev);
    int expectedLba = physicalSize.startLba + (src.stripeId * physicalSize.blksPerChunk + src.offset % physicalSize.blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;
    ASSERT_EQ(expectedLba, dest.lba);
}

TEST(StripePartition, Convert_testIfConvertWithRaid1FillsDestIn)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .startLba = 5,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    Raid1* raid1 = new Raid1(&physicalSize);
    vector<ArrayDevice*> devs;
    for (int i = 0; i < physicalSize.chunksPerStripe; i += 1)
    {
        devs.push_back(new ArrayDevice(nullptr));
    }
    StripePartition sPartition("mock-array", PartitionType::USER_DATA, physicalSize, devs, raid1);
    std::list<BufferEntry> buffers;
    LogicalBlkAddr lBlkAddr{
        .stripeId = 5,
        .offset = 2};
    const LogicalWriteEntry src{
        .addr = lBlkAddr,
        .blkCnt = 4,
        .buffers = &buffers};
    list<PhysicalWriteEntry> dest;

    // When
    int actual = sPartition.Convert(dest, src);

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(2, dest.size());
    ASSERT_EQ(src.blkCnt, dest.front().blkCnt);
    dest.pop_front();
    ASSERT_EQ(src.blkCnt, dest.front().blkCnt);
}

TEST(StripePartition, GetRecoverMethod_testIfValidUbioCanRetrieveRecoverMethodSuccessfully)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    Raid1* raid1 = new Raid1(&physicalSize);
    ArrayDevice arrayDevice1(nullptr), arrayDevice2(nullptr), arrayDevice3(nullptr), arrayDevice4(nullptr);
    vector<ArrayDevice*> devs = {&arrayDevice1, &arrayDevice2, &arrayDevice3, &arrayDevice4};
    StripePartition sPartition("mock-array", PartitionType::USER_DATA, physicalSize, devs, raid1);

    int DATA_BUFFER_UNIT = 13; // just picked up randomly
    UbioSmartPtr ubio = make_shared<Ubio>(nullptr, DATA_BUFFER_UNIT, "mock-array");
    uint64_t LBA = physicalSize.startLba;
    ubio->SetLba(LBA);
    PhysicalBlkAddr pba{
        .lba = LBA,
        .arrayDev = &arrayDevice1};
    ubio->SetPba(pba);

    RecoverMethod out;

    // When
    int actual = sPartition.GetRecoverMethod(ubio, out);

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
    int expectedBlockAlignedLba = 0;
    StripeId expectedFtBlkAddrStripeId = 0;
    int expectedFtBlkAddrOffset = 0;
    // int expectedMirroDeviceCount = physicalSize.chunksPerStripe / 2; // becomes 2 when we have 4 devices, which may or may not be realistic though
    int expectedPhysicalLBA = physicalSize.startLba + (expectedFtBlkAddrStripeId * physicalSize.blksPerChunk + expectedFtBlkAddrOffset % physicalSize.blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;
    ASSERT_EQ(1, out.srcAddr.size());
    ASSERT_EQ(expectedPhysicalLBA, out.srcAddr.front().lba);
}

TEST(StripePartition, GetRebuildCtx_testIfRebuildContextIsFilledInWithFaultyDevice)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    Raid1* raid1 = new Raid1(&physicalSize);
    ArrayDevice arrayDevice1(nullptr), arrayDevice2(nullptr), arrayDevice3(nullptr);
    ArrayDevice arrayDevice4(nullptr, ArrayDeviceState::FAULT); // it seems current GetRebuildCtx() doesn't care about the state.
    vector<ArrayDevice*> devs = {&arrayDevice1, &arrayDevice2, &arrayDevice3, &arrayDevice4};
    PartitionType PART_TYPE = PartitionType::USER_DATA;
    string ARRAY_NAME = "mock-array";
    StripePartition sPartition(ARRAY_NAME, PART_TYPE, physicalSize, devs, raid1);

    // When
    auto rebuildCtx = sPartition.GetRebuildCtx(&arrayDevice4);

    // Then
    ASSERT_TRUE(rebuildCtx != nullptr);
    ASSERT_TRUE(raid1->GetRaidType() == rebuildCtx->raidType);
    ASSERT_TRUE(RebuildState::READY == rebuildCtx->result); // gtest doesn't allow the use of enum class?
    ASSERT_EQ(3, rebuildCtx->faultIdx);
    ASSERT_EQ(ARRAY_NAME, rebuildCtx->array);
    int expectedLogicalTotalStripes = sPartition.GetLogicalSize()->totalStripes;
    ASSERT_EQ(expectedLogicalTotalStripes, rebuildCtx->stripeCnt);
}

TEST(StripePartition, Format_testIfThereAre4DeallocatesAnd4ReadsWhenThereAre4NormalDevices)
{
    // Given
    PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 30};
    Raid1* raid1 = new Raid1(&physicalSize);
    shared_ptr<MockUBlockDevice> ublockDev1Shared = make_shared<MockUBlockDevice>("", 0, nullptr);
    shared_ptr<MockUBlockDevice> ublockDev2Shared = make_shared<MockUBlockDevice>("", 0, nullptr);
    shared_ptr<MockUBlockDevice> ublockDev3Shared = make_shared<MockUBlockDevice>("", 0, nullptr);
    shared_ptr<MockUBlockDevice> ublockDev4Shared = make_shared<MockUBlockDevice>("", 0, nullptr);
    ArrayDevice arrayDevice1(ublockDev1Shared),
        arrayDevice2(ublockDev2Shared),
        arrayDevice3(ublockDev3Shared),
        arrayDevice4(ublockDev4Shared);
    vector<ArrayDevice*> devs = {&arrayDevice1, &arrayDevice2, &arrayDevice3, &arrayDevice4};
    PartitionType PART_TYPE = PartitionType::USER_DATA;
    string ARRAY_NAME = "mock-array";
    MockIODispatcher mockIoDispatcher;
    StripePartition sPartition(ARRAY_NAME, PART_TYPE, physicalSize, devs, raid1, &mockIoDispatcher);

    EXPECT_CALL(*ublockDev1Shared.get(), GetName).WillRepeatedly(Return("dev1"));
    EXPECT_CALL(*ublockDev2Shared.get(), GetName).WillRepeatedly(Return("dev2"));
    EXPECT_CALL(*ublockDev3Shared.get(), GetName).WillRepeatedly(Return("dev3"));
    EXPECT_CALL(*ublockDev4Shared.get(), GetName).WillRepeatedly(Return("dev4"));

    int cntSubmitIo = 0;
    EXPECT_CALL(mockIoDispatcher, Submit).WillRepeatedly([&physicalSize, &cntSubmitIo](UbioSmartPtr ubio, bool sync, bool ublockSharedPtrCopyNeeded)
    {
        if (cntSubmitIo < 4)
        {
            if (ubio->dir != UbioDir::Deallocate)
            {
                return -1;
            }
        }
        else
        {
            if (ubio->dir != UbioDir::Read)
            {
                return -1;
            }
        }

        if (sync == false)
        {
            return -1;
        }

        if (ubio->GetLba() != physicalSize.startLba)
        {
            return -1;
        }

        cntSubmitIo += 1;
        return 0;
    });

    // When
    sPartition.Format();

    // Then
}

} // namespace pos
