#include "src/array/partition/nvm_partition.h"

#include <gtest/gtest.h>

#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/helper/calc/calc.h"
#include "test/unit-tests/array/ft/buffer_entry_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"

using ::testing::Return;
namespace pos
{
static LogicalBlkAddr
buildValidLogicalBlkAddr(uint32_t totalStripes, uint32_t blksPerStripe)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes / 2,
        .offset = 0};
    return lBlkAddr;
}

static LogicalBlkAddr
buildInvalidLogicalBlkAddr(uint32_t totalStripes)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes + 1,
        .offset = 0};
    return lBlkAddr;
}

static LogicalWriteEntry
buildValidLogicalWriteEntry(uint32_t totalStripes, uint32_t blksPerStripe)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes / 2,
        .offset = 0};

    std::list<BufferEntry>* fakeBuffers = new std::list<BufferEntry>;
    MockBufferEntry mockBuffer(nullptr, 0, false);
    fakeBuffers->push_back(mockBuffer);

    LogicalWriteEntry lWriteEntry{
        .addr = lBlkAddr,
        .blkCnt = blksPerStripe / 2,
        .buffers = fakeBuffers};

    return lWriteEntry;
}

static LogicalByteWriteEntry
buildValidLogicalByteWriteEntry(uint32_t totalStripes, uint32_t byteCnt)
{
    LogicalBlkAddr lBlkAddr
    {
        .stripeId = totalStripes / 2,
        .offset = 0
    };

    LogicalByteAddr lByteAddr
    {
        .blkAddr = lBlkAddr,
        .byteOffset = 0,
        .byteSize = byteCnt
    };

    std::list<BufferEntry>* fakeBuffers = new std::list<BufferEntry>;
    MockBufferEntry mockBuffer(nullptr, 0, false);
    fakeBuffers->push_back(mockBuffer);

    LogicalByteWriteEntry lWriteEntry
    {
        .addr = lByteAddr,
        .byteCnt = byteCnt,
        .buffers = fakeBuffers
    };

    return lWriteEntry;
}

static LogicalWriteEntry
buildInvalidLogicalWriteEntry(uint32_t totalStripes, uint32_t blksPerStripe)
{
    LogicalBlkAddr lBlkAddr{
        .stripeId = totalStripes / 2,
        .offset = 0};

    LogicalWriteEntry lWriteEntry{
        .addr = lBlkAddr,
        .blkCnt = blksPerStripe + 1, // intentionally making it too big
        .buffers = nullptr};

    return lWriteEntry;
}

TEST(NvmPartition, NvmPartition_testIfCreateMetaNvmInitializesPhysicalAndLogicalSizeProperly)
{
    // Given
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint64_t startLba = 0; // not interesting
    uint32_t blksPerChunk = 4096; // not interesting
    // When
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);
    // Then
    uint32_t devcnt = ArrayConfig::NVM_DEVICE_COUNT;
    uint64_t metaNvmSize = ArrayConfig::META_NVM_SIZE;
    uint32_t blkSizeByte = ArrayConfig::BLOCK_SIZE_BYTE;
    uint32_t nvmSegSize = ArrayConfig::NVM_SEGMENT_SIZE;

    const PartitionPhysicalSize* pPhysicalSize = nvmPart.GetPhysicalSize();
    ASSERT_EQ(startLba, pPhysicalSize->startLba);
    ASSERT_EQ(blksPerChunk, pPhysicalSize->blksPerChunk);
    ASSERT_EQ(devcnt, pPhysicalSize->chunksPerStripe);
    ASSERT_EQ(nvmSegSize, pPhysicalSize->totalSegments);
    ASSERT_EQ(metaNvmSize / (blkSizeByte * blksPerChunk * devcnt), pPhysicalSize->stripesPerSegment);

    const PartitionLogicalSize* pLogicalSize = nvmPart.GetLogicalSize();
    ASSERT_EQ(1, pLogicalSize->minWriteBlkCnt);
    ASSERT_EQ(pPhysicalSize->blksPerChunk, pLogicalSize->blksPerChunk);
    ASSERT_EQ(pPhysicalSize->blksPerChunk * pPhysicalSize->chunksPerStripe, pLogicalSize->blksPerStripe);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment * pPhysicalSize->totalSegments, pLogicalSize->totalStripes);
    ASSERT_EQ(pPhysicalSize->totalSegments, pLogicalSize->totalSegments);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment, pLogicalSize->stripesPerSegment);
}

TEST(NvmPartition, NvmPartition_testIfCreateWriteBufferInitializesPhysicalAndLogicalSizeProperly)
{
    // Given
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 1024; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint64_t startLba = 0; // not interesting
    uint32_t blksPerChunk = 4096; // not interesting
    // When
    NvmPartition nvmPart(PartitionType::WRITE_BUFFER, devs);
    nvmPart.Create(startLba, blksPerChunk);
    // Then
    uint32_t devcnt = ArrayConfig::NVM_DEVICE_COUNT;
    uint64_t sectorsPerBlk = ArrayConfig::SECTORS_PER_BLOCK;
    uint32_t blkSizeByte = ArrayConfig::BLOCK_SIZE_BYTE;
    uint32_t nvmSegSize = ArrayConfig::NVM_SEGMENT_SIZE;

    const PartitionPhysicalSize* pPhysicalSize = nvmPart.GetPhysicalSize();
    ASSERT_EQ(startLba, pPhysicalSize->startLba);
    ASSERT_EQ(blksPerChunk, pPhysicalSize->blksPerChunk);
    ASSERT_EQ(devcnt, pPhysicalSize->chunksPerStripe);
    ASSERT_EQ(nvmSegSize, pPhysicalSize->totalSegments);
    uint32_t expectStrPerSeg = (nvmSize / blkSizeByte - DIV_ROUND_UP(startLba, sectorsPerBlk)) / blksPerChunk;
    ASSERT_EQ(expectStrPerSeg, pPhysicalSize->stripesPerSegment);

    const PartitionLogicalSize* pLogicalSize = nvmPart.GetLogicalSize();
    ASSERT_EQ(1, pLogicalSize->minWriteBlkCnt);
    ASSERT_EQ(pPhysicalSize->blksPerChunk, pLogicalSize->blksPerChunk);
    ASSERT_EQ(pPhysicalSize->blksPerChunk * pPhysicalSize->chunksPerStripe, pLogicalSize->blksPerStripe);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment * pPhysicalSize->totalSegments, pLogicalSize->totalStripes);
    ASSERT_EQ(pPhysicalSize->totalSegments, pLogicalSize->totalSegments);
    ASSERT_EQ(pPhysicalSize->stripesPerSegment, pLogicalSize->stripesPerSegment);
}

TEST(NvmPartition, Translate_testIfInvalidAddressReturnsError)
{
    // Given
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint64_t startLba = 0; // not interesting
    uint32_t blksPerChunk = 4096; // not interesting
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);
    LogicalEntry invalidAddr;
    LogicalBlkAddr invalidBlkAddr = buildInvalidLogicalBlkAddr(nvmPart.GetLogicalSize()->totalStripes);
    invalidAddr.addr.stripeId = invalidBlkAddr.stripeId;
    invalidAddr.addr.offset = invalidBlkAddr.offset;
    invalidAddr.blkCnt = 1;
    list<PhysicalEntry> ignored;

    // When
    int actual = nvmPart.Translate(ignored, invalidAddr);

    // Then
    ASSERT_EQ(EID(ADDRESS_TRANSLATION_INVALID_LBA), actual);
}

TEST(NvmPartition, Translate_testIfValidAddressIsFilledIn)
{
    // Given
    uint64_t startLba = 0; // not interesting
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint32_t blksPerChunk = 4096; // not interesting
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);
    uint32_t totalStripes = nvmPart.GetLogicalSize()->totalStripes;
    LogicalEntry validAddr;
    validAddr.addr  = buildValidLogicalBlkAddr(totalStripes,
        nvmPart.GetLogicalSize()->blksPerStripe);
    validAddr.blkCnt = 1;
    list<PhysicalEntry> dest;
    // When
    int actual = nvmPart.Translate(dest, validAddr);

    // Then
    ASSERT_EQ(0, actual);
    int expectedSrcBlock = totalStripes / 2 * nvmPart.GetLogicalSize()->blksPerStripe;
    int expectedSrcSector = expectedSrcBlock * ArrayConfig::SECTORS_PER_BLOCK;
    int expectedDestSector = expectedSrcSector + startLba;
    ASSERT_EQ(expectedDestSector, dest.front().addr.lba);
}

TEST(NvmPartition, ByteTranslate_testIfInvalidAddressReturnsError)
{
    // Given
    uint64_t startLba = 0; // not interesting
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint32_t blksPerChunk = 4096; // not interesting
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);

    uint32_t totalStripes = nvmPart.GetLogicalSize()->totalStripes;
    uint32_t blksPerStripe = nvmPart.GetLogicalSize()->blksPerChunk;
    uint32_t testByteOffset = 10;
    LogicalBlkAddr logicalByteAddr = buildInvalidLogicalBlkAddr(totalStripes);
    LogicalByteAddr invalidAddr;
    invalidAddr.blkAddr = logicalByteAddr;
    invalidAddr.byteOffset = testByteOffset;
    PhysicalByteAddr ignored;

    // When
    int actual = nvmPart.ByteTranslate(ignored, invalidAddr);

    // Then
    ASSERT_EQ(EID(ADDRESS_BYTE_TRANSLATION_INVALID_LBA), actual);
}

TEST(NvmPartition, ByteTranslate_testIfValidAddressIsFilledIn)
{
    // Given
    uint64_t startLba = 0; // not interesting
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    EXPECT_CALL(*mockUblock, GetByteAddress).WillOnce(Return((void*)0));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint32_t blksPerChunk = 4096; // not interesting
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);
    uint32_t totalStripes = nvmPart.GetLogicalSize()->totalStripes;
    uint32_t blksPerStripe = nvmPart.GetLogicalSize()->blksPerChunk;
    LogicalBlkAddr logicalblkAddr = buildValidLogicalBlkAddr(totalStripes, blksPerStripe);
    uint32_t testByteOffset = 5;
    LogicalByteAddr validAddr;
    validAddr.blkAddr = logicalblkAddr;
    validAddr.byteOffset = testByteOffset;
    validAddr.byteSize = 10;
    PhysicalByteAddr dest;

    // When
    int actual = nvmPart.ByteTranslate(dest, validAddr);

    // Then
    ASSERT_EQ(0, actual);
    int expectedSrcBlock = totalStripes / 2 * nvmPart.GetLogicalSize()->blksPerStripe;
    int expectedSrcSector = expectedSrcBlock * ArrayConfig::SECTORS_PER_BLOCK;
    int expectedDestByte = (expectedSrcSector + startLba) * ArrayConfig::SECTOR_SIZE_BYTE + testByteOffset;
    ASSERT_EQ(expectedDestByte, dest.byteAddress);
}

TEST(NvmPartition, GetParityList_testIfValidEntryIsFilledIn)
{
    // Given
    uint64_t startLba = 0; // not interesting
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint32_t blksPerChunk = 4096; // not interesting
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);
    uint32_t totalStripes = nvmPart.GetLogicalSize()->totalStripes;
    uint32_t blksPerStripe = nvmPart.GetLogicalSize()->blksPerChunk;
    LogicalWriteEntry validEntry = buildValidLogicalWriteEntry(totalStripes, blksPerStripe);
    std::list<PhysicalWriteEntry> dest;

    // When
    int actual = nvmPart.GetParityList(dest, validEntry);

    // Then
    ASSERT_EQ(0, dest.size());
}

TEST(NvmPartition, ByteConvert_testIfValidEntryIsFilledIn)
{
    // Given
    uint64_t startLba = 0; // not interesting
    vector<ArrayDevice*> devs;
    uint64_t nvmSize = 1024 * 1024 * 4; // not interesting
    string nvmName = "uram0";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(nvmName, nvmSize, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(nvmName.c_str()));
    EXPECT_CALL(*mockUblock, GetSize).WillRepeatedly(Return(nvmSize));
    EXPECT_CALL(*mockUblock, GetByteAddress).WillOnce(Return((void*)0));
    ArrayDevice nvm(mockUblock);
    devs.push_back(&nvm);
    uint32_t blksPerChunk = 4096; // not interesting
    NvmPartition nvmPart(PartitionType::META_NVM, devs);
    nvmPart.Create(startLba, blksPerChunk);
    uint32_t totalStripes = nvmPart.GetLogicalSize()->totalStripes;
    uint32_t blksPerStripe = nvmPart.GetLogicalSize()->blksPerChunk;
    uint32_t blkCnt = 10; // bytes to convert
    LogicalByteWriteEntry validEntry = buildValidLogicalByteWriteEntry(totalStripes, blkCnt);
    std::list<PhysicalByteWriteEntry> dest;

    // When
    int actual = nvmPart.ByteConvert(dest, validEntry);

    // Then
    ASSERT_EQ(1, dest.size());
    PhysicalByteWriteEntry pWriteEntry = dest.front();
    ASSERT_EQ(validEntry.byteCnt, pWriteEntry.byteCnt);
    PhysicalByteAddr phyByteAddr = pWriteEntry.addr;
}

TEST(NvmPartition, IsByteAccessSupported_testIfReturnValueCorrect)
{
    // Given
    vector<ArrayDevice*> devs;
    devs.push_back(nullptr); // 'cause I'm not interested
    NvmPartition nvmPart(PartitionType::META_NVM, devs);

    // When
    bool actual = nvmPart.IsByteAccessSupported();

    // Then
    ASSERT_TRUE(actual);
}

TEST(NvmPartition, Include_testCopyOperatorOfIncludedStructure)
{
    // Given
    PhysicalWriteEntry pwe1;
    PhysicalWriteEntry pwe2;
    // When
    pwe1 = pwe2;
}

} // namespace pos
