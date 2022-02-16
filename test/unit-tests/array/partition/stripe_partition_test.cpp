#include "src/array/partition/stripe_partition.h"

#include <gtest/gtest.h>

#include "src/array/device/array_device.h"
#include "src/array/ft/raid5.h"
#include "src/array/ft/raid10.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/bio/ubio.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/helper/calc/calc.h"
#include "test/unit-tests/array/ft/raid5_mock.h"
#include "test/unit-tests/array/ft/raid10_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"

using ::testing::Return;

namespace pos
{

TEST(StripePartition, StripePartition_testIfCreateUserDataWithRaid5InitializesPhysicalAndLogicalSizeProperly)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    uint64_t startLba = 1024; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    uint64_t ssdTotalSegments = devSize / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    uint64_t metaSegments = DIV_ROUND_UP(ssdTotalSegments * ArrayConfig::META_SSD_SIZE_RATIO, (uint64_t)(100));
    uint64_t mbrSegments = ArrayConfig::MBR_SIZE_BYTE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;

    // When
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, ssdTotalSegments - mbrSegments - metaSegments, totalNvmBlks);

    // Then
    uint64_t stripesPerSeg = ArrayConfig::STRIPES_PER_SEGMENT;
    uint32_t blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;

    const PartitionPhysicalSize* pPhysicalSize = sPartition.GetPhysicalSize();
    ASSERT_EQ(startLba, pPhysicalSize->startLba);
    ASSERT_EQ(blksPerChunk, pPhysicalSize->blksPerChunk);
    ASSERT_EQ(devCnt, pPhysicalSize->chunksPerStripe);
    ASSERT_EQ(stripesPerSeg, pPhysicalSize->stripesPerSegment);
    ASSERT_EQ(ssdTotalSegments - metaSegments - mbrSegments, pPhysicalSize->totalSegments);

    const PartitionLogicalSize* pLogicalSize = sPartition.GetLogicalSize();
    ASSERT_EQ(pPhysicalSize->blksPerChunk, pLogicalSize->blksPerChunk);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment, pLogicalSize->stripesPerSegment);
    ASSERT_EQ(pPhysicalSize->totalSegments, pLogicalSize->totalSegments);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment * pPhysicalSize->totalSegments, pLogicalSize->totalStripes);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, StripePartition_testIfCreateMetaSsdWithRaid1InitializesPhysicalAndLogicalSizeProperly)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    uint32_t devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    uint64_t startLba = 0; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID10;
    uint64_t ssdTotalSegments = devSize / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    uint64_t metaSegments = DIV_ROUND_UP(ssdTotalSegments * ArrayConfig::META_SSD_SIZE_RATIO, (uint64_t)(100));

    // When
    StripePartition sPartition(PartitionType::META_SSD, devs, raid);
    sPartition.Create(startLba, metaSegments, totalNvmBlks);

    // Then
    uint64_t stripesPerSeg = ArrayConfig::STRIPES_PER_SEGMENT;
    uint32_t blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;
    uint32_t actualDevCnt = sPartition.GetDevs().size();
    const PartitionPhysicalSize* pPhysicalSize = sPartition.GetPhysicalSize();
    ASSERT_EQ(startLba, pPhysicalSize->startLba);
    ASSERT_EQ(blksPerChunk, pPhysicalSize->blksPerChunk);
    ASSERT_EQ(actualDevCnt, pPhysicalSize->chunksPerStripe);
    ASSERT_EQ(stripesPerSeg, pPhysicalSize->stripesPerSegment);
    ASSERT_EQ(metaSegments , pPhysicalSize->totalSegments);

    const PartitionLogicalSize* pLogicalSize = sPartition.GetLogicalSize();
    ASSERT_EQ(pPhysicalSize->blksPerChunk, pLogicalSize->blksPerChunk);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment, pLogicalSize->stripesPerSegment);
    ASSERT_EQ(pPhysicalSize->totalSegments, pLogicalSize->totalSegments);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment * pPhysicalSize->totalSegments, pLogicalSize->totalStripes);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, Translate_testIfFtBlkAddrIsMappedToPhysicalBlkAddr)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    uint64_t startLba = 0; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID10;
    StripePartition sPartition(PartitionType::META_SSD, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
    auto psize = sPartition.GetPhysicalSize();
    auto lsize = sPartition.GetLogicalSize();
    StripeId MAX_BLK_OFFSET = psize->blksPerChunk * psize->chunksPerStripe - 1;
    LogicalBlkAddr srcaddr{
        .stripeId = lsize->totalStripes - 1,
        .offset = MAX_BLK_OFFSET / 2};
    LogicalEntry src{
        .addr = srcaddr,
        .blkCnt = 1
    };
    list<PhysicalEntry> dest;

    // When
    int actual = sPartition.Translate(dest, src);

    // Then
    ASSERT_EQ(0, actual);
    int expectedChunkIndex = src.addr.offset / psize->blksPerChunk;
    ASSERT_EQ(devs.at(expectedChunkIndex), dest.front().addr.arrayDev);
    int expectedLba = psize->startLba + (src.addr.stripeId * psize->blksPerChunk + src.addr.offset % psize->blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;
    ASSERT_EQ(expectedLba, dest.front().addr.lba);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, GetParityList_testIfParityWithRaid10FillsDestIn)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    uint64_t startLba = 0; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID10;
    StripePartition sPartition(PartitionType::META_SSD, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
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
    int actual = sPartition.GetParityList(dest, src);

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(1, dest.size());
    ASSERT_EQ(src.blkCnt, dest.front().blkCnt);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, GetRecoverMethod_testIfValidUbioCanRetrieveRecoverMethodSuccessfully)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    uint64_t startLba = 0; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID10;
    StripePartition sPartition(PartitionType::META_SSD, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);

    int DATA_BUFFER_UNIT = 13; // just picked up randomly
    UbioSmartPtr ubio = make_shared<Ubio>(nullptr, DATA_BUFFER_UNIT, "mock-array");
    uint64_t LBA = startLba;
    ubio->SetLba(LBA);
    PhysicalBlkAddr pba{
        .lba = LBA,
        .arrayDev = devs[0]};
    ubio->SetPba(pba);

    RecoverMethod out;

    // When
    int actual = sPartition.GetRecoverMethod(ubio, out);
    auto physicalSize = sPartition.GetPhysicalSize();

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
    int expectedBlockAlignedLba = 0;
    StripeId expectedFtBlkAddrStripeId = 0;
    int expectedFtBlkAddrOffset = 0;
    // int expectedMirroDeviceCount = physicalSize.chunksPerStripe / 2; // becomes 2 when we have 4 devices, which may or may not be realistic though
    int expectedPhysicalLBA = physicalSize->startLba + (expectedFtBlkAddrStripeId * physicalSize->blksPerChunk + expectedFtBlkAddrOffset % physicalSize->blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;
    ASSERT_EQ(1, out.srcAddr.size());
    ASSERT_EQ(expectedPhysicalLBA, out.srcAddr.front().lba);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, GetRebuildCtx_testIfRebuildContextIsFilledInWithFaultyDevice)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    ArrayDevice* arrayDevice4 = new ArrayDevice(nullptr, ArrayDeviceState::FAULT); // it seems current GetRebuildCtx() doesn't care about the state.
    devs.push_back(arrayDevice4); // it seems current GetRebuildCtx() doesn't care about the state.
    uint64_t startLba = 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);

    // When
    auto rebuildCtx = sPartition.GetRebuildCtx(arrayDevice4);

    // Then
    ASSERT_TRUE(rebuildCtx != nullptr);
    ASSERT_TRUE(RebuildState::READY == rebuildCtx->GetResult()); // gtest doesn't allow the use of enum class?
    ASSERT_EQ(3, rebuildCtx->faultIdx);
    int expectedLogicalTotalStripes = sPartition.GetLogicalSize()->totalStripes;
    ASSERT_EQ(expectedLogicalTotalStripes, rebuildCtx->stripeCnt);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, Format_testIfThereAre4DeallocatesAnd4ReadsWhenThereAre4NormalDevices)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    uint64_t startLba = 1024; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
    auto physicalSize = sPartition.GetPhysicalSize();
    MockIODispatcher mockIoDispatcher;
    int cntSubmitIo = 0;
    EXPECT_CALL(mockIoDispatcher, Submit).WillRepeatedly([physicalSize, &cntSubmitIo](UbioSmartPtr ubio, bool sync, bool ublockSharedPtrCopyNeeded)
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

        if (ubio->GetLba() != physicalSize->startLba)
        {
            return -1;
        }

        cntSubmitIo += 1;
        return 0;
    });

    // When

    // Then

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, ByteTranslate_testFunctionCallForCoverage)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }

    uint64_t startLba = 1024; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
    // When
    PhysicalByteAddr dst;
    LogicalByteAddr src;
    int result = sPartition.ByteTranslate(dst, src);
    // Then
    int BYTE_TRANSLATE_NOT_SUPPORTED = -1;
    EXPECT_EQ(BYTE_TRANSLATE_NOT_SUPPORTED, result);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, ByteConvert_testFunctionCallForCoverage)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }

    uint64_t startLba = 1024; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
    // When
    list<PhysicalByteWriteEntry> dst;
    LogicalByteWriteEntry src;
    int result = sPartition.ByteConvert(dst, src);
    // Then
    int BYTE_CONVERT_NOT_SUPPORTED = -1;
    EXPECT_EQ(BYTE_CONVERT_NOT_SUPPORTED, result);
    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, IsByteAccessSupported_testReturnValue)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }

    uint64_t startLba = 1024; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
    // When
    bool result = sPartition.IsByteAccessSupported();
    // Then
    EXPECT_FALSE(result);

    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

TEST(StripePartition, GetRaidState_testIfExtractDeviceStateListFromArrayDeviceList)
{
    // Given
    vector<ArrayDevice*> devs;
    string devNamePrefix = "unvme-ns-"; // not interesting
    uint64_t devSize = 1024 * 1024 * 1024; // not interesting
    int devCnt = 3;
    for (int i = 0; i < devCnt; i++)
    {
        string devName = devNamePrefix + to_string(i);
        shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, devSize, nullptr);
        EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
        EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(devSize));
        ArrayDevice* dev = new ArrayDevice(mockUblock, ArrayDeviceState::NORMAL);
        devs.push_back(dev);
    }
    devs[1]->SetState(ArrayDeviceState::FAULT); // to make state as degraded.

    uint64_t startLba = 1024; // not interesting
    uint32_t totalNvmBlks = 1024 * 1024; // not interesting
    uint32_t segCnt = 1; // not interesting
    RaidTypeEnum raid = RaidTypeEnum::RAID5;
    StripePartition sPartition(PartitionType::USER_DATA, devs, raid);
    sPartition.Create(startLba, segCnt, totalNvmBlks);
    // When
    RaidState state = sPartition.GetRaidState();
    // Then
    ASSERT_EQ(RaidState::DEGRADED, state);
    // Wrap up
    for (auto dev : devs)
    {
        delete dev;
    }
}

} // namespace pos
