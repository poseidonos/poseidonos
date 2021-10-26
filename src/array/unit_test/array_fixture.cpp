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
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/array/array.h"
#include "src/device/device_manager_mock.h"
#include "src/master_context/mbr_manager_mock.h"

namespace
{
class ArrayFixture : public testing::Test
{
protected:
    virtual void
    SetUp()
    {
        Array::ResetInstance();
        //        MbrManager* mbrMgr = new MockMbrManager;
        //        DeviceManager* devMgr = new MockDeviceManager;
        //        sysArray = Array::Instance(mbrMgr, devMgr);
        //
        //        DeviceSet<DevName> nameSet = GetNameSet();
        //        sysArray->Create(nameSet);
        //        sysArray->Mount();
    }

    virtual void
    TearDown()
    {
        //        sysArray->Unmount();
        //        sysArray->Delete();
    }

    DeviceSet<DevName>
    GetNameSet()
    {
        const string NVM_NAME = "nvm";
        const string DATA_SSD_NAME = "DATA_";
        const string SPARE_SSD_NAME = "SPARE_";

        DeviceSet<DevName> nameSet;
        nameSet.nvm = DevName(NVM_NAME);
        for (uint32_t i = 0; i < DATA_DEVICE_COUNT; i++)
        {
            nameSet.data.push_back(DevName(DATA_SSD_NAME + to_string(i)));
        }
        for (uint32_t i = 0; i < SPARE_DEVICE_COUNT; i++)
        {
            nameSet.data.push_back(DevName(SPARE_SSD_NAME + to_string(i)));
        }
        return nameSet;
    }

    const uint32_t DATA_DEVICE_COUNT = 3;
    const uint32_t SPARE_DEVICE_COUNT = 1;
    const uint64_t NVM_SIZE = 2 * 1024 * 256;  // 2GB(4K BlockCount)
    const uint64_t SSD_SIZE = 32 * 1024 * 256; // 32GB(4K BlockCount)

    Array* sysArray;

private:
};

TEST_F(ArrayFixture, Load)
{
    MockMbrManager mbrMgr;
    MbrManager* mbr = &mbrMgr;
    EXPECT_CALL(mbrMgr, GetValue("nvm_size")).WillRepeatedly(Return("1"));
    EXPECT_CALL(mbr, GetValue("nvm_size")).WillRepeatedly(Return("1"));
    EXPECT_CALL(mbrMgr, GetValue("nvm_uid_0")).WillRepeatedly(Return("NVM_UID_0"));
    EXPECT_CALL(mbrMgr, GetValue("data_size")).WillRepeatedly(Return("3"));
    EXPECT_CALL(mbrMgr, GetValue("data_uid_0")).WillRepeatedly(Return("DATA_UID_0"));
    EXPECT_CALL(mbrMgr, GetValue("data_uid_1")).WillRepeatedly(Return("DATA_UID_1"));
    EXPECT_CALL(mbrMgr, GetValue("data_uid_2")).WillRepeatedly(Return("DATA_UID_2"));
    EXPECT_CALL(mbrMgr, GetValue("spare_size")).WillRepeatedly(Return("1"));
    EXPECT_CALL(mbrMgr, GetValue("spare_uid_0")).WillRepeatedly(Return("SPARE_UID_0"));

    cout << mbrMgr.GetValue("nvm_size") << endl;
    cout << mbr->GetValue("nvm_size") << endl;

    DeviceManager* devMgr = new MockDeviceManager;

    sysArray = Array::Instance(&mbrMgr, devMgr);

    int ret = sysArray->Load();
    EXPECT_EQ(0, ret);
}

// TEST_F(ArrayFixture, Create)
//{
//    sysArray->Unmount();
//    sysArray->Delete();
//    int ret = sysArray->Create(nameSet);
//    EXPECT_EQ(0, ret);
//}
//
// TEST_F(ArrayFixture, Mount)
//{
//    sysArray->Unmount();
//    int ret = sysArray->Mount();
//    EXPECT_EQ(0, ret);
//}
//
// TEST_F(ArrayFixture, Unmount)
//{
//    int ret = sysArray->Unmount();
//    EXPECT_EQ(0,ret);
//}
//
// TEST_F(ArrayFixture, Delete)
//{
//    sysArray->Unmount();
//    int ret = sysArray->Delete();
//    EXPECT_EQ(0,ret);
//}
//
// TEST_F(ArrayFixture, AddSpare)
//{
//    UblockSharedPtr spare1 = new MockUBlockDevice("SPARE_1", SSD_SIZE);
//    int ret = sysArray->AddSpare(spare1);
//    EXPECT_EQ(0, ret);
//    sysArray->Unmount();
//    UblockSharedPtr spare2 = new MockUBlockDevice("SPARE_2", SSD_SIZE);
//    ret = sysArray->AddSpare(spare2);
//    EXPECT_EQ(0, ret);
//}
//
// TEST_F(ArrayFixture, GetStatus)
//{
//    ArrayStatus status = sysArray->GetStatus();
//    EXPECT_EQ(ArrayStatus::MOUNTED, status);
//    sysArray->Unmount();
//    status = sysArray->GetStatus();
//    EXPECT_EQ(ArrayStatus::CREATED, status);
//    sysArray->Delete();
//    status = sysArray->GetStatus();
//    EXPECT_EQ(ArrayStatus::DELETED, status);
//}
//
//
// TEST_F(ArrayFixture, GetSizeInfo)
//{
//    const PartitionLogicalSize* size =
//        sysArray->GetSizeInfo(PartitionType::WRITE_BUFFER);
//    EXPECT_EQ(1, size->minWriteBlkCnt);
//    EXPECT_EQ(128, size->blksPerStripe);
//    EXPECT_EQ(1, size->chunksPerStripe);
//    EXPECT_EQ(3072, size->stripesPerSegment);
//    EXPECT_EQ(3072, size->totalStripes);
//    EXPECT_EQ(1, size->totalSegments);
//
//    size = sysArray->GetSizeInfo(PartitionType::USER_DATA);
//    EXPECT_EQ(192, size->minWriteBlkCnt);
//    EXPECT_EQ(128, size->blksPerStripe);
//    EXPECT_EQ(DATA_DEVICE_COUNT - 1, size->chunksPerStripe);
//    EXPECT_EQ(1024, size->stripesPerSegment);
//    EXPECT_EQ(131072, size->totalStripes);
//    EXPECT_EQ(128, size->totalSegments);
//
//    size = sysArray->GetSizeInfo(PartitionType::META_NVM);
//    EXPECT_EQ(1, size->minWriteBlkCnt);
//    EXPECT_EQ(64, size->blksPerStripe);
//    EXPECT_EQ(1, size->chunksPerStripe);
//    EXPECT_EQ(2048, size->stripesPerSegment);
//    EXPECT_EQ(2048, size->totalStripes);
//    EXPECT_EQ(1, size->totalSegments);
//
//    size = sysArray->GetSizeInfo(PartitionType::META_SSD);
//    EXPECT_EQ(1, size->minWriteBlkCnt);
//    EXPECT_EQ(64, size->blksPerStripe);
//    EXPECT_EQ(1, size->chunksPerStripe);
//    EXPECT_EQ(1024, size->stripesPerSegment);
//    EXPECT_EQ(1024, size->totalStripes);
//    EXPECT_EQ(1, size->totalSegments);
//}
//
// TEST_F(ArrayFixture, Translate)
//{
//    //META_NVM
//    LogicalBlkAddr lsa = { .stripeId = 0, .offset = 0 };
//    PhysicalBlkAddr psa = sysArray->Translate(PartitionType::META_NVM,lsa);
//    EXPECT_EQ(uBlockSet.nvm, psa.dev);
//    EXPECT_EQ(0, psa.lba);
//
//    //WRITE_BUFFER
//    psa = sysArray->Translate(PartitionType::WRITE_BUFFER,lsa);
//    EXPECT_EQ(uBlockSet.nvm, psa.dev);
//    const PartitionLogicalSize* mnSize =
//        sysArray->GetSizeInfo(PartitionType::META_NVM);
//    uint64_t lba = mnSize->blksPerStripe * mnSize->totalStripes
//        * ArrayConfig::SECTORS_PER_BLOCK;
//    EXPECT_EQ(lba, psa.lba);
//
//    //META_SSD
//    lsa.offset += ArrayConfig::BLOCKS_PER_CHUNK;
//    psa = sysArray->Translate(PartitionType::META_SSD,lsa);
//    EXPECT_EQ(*(uBlockSet.data.begin()+1), psa.dev);
//    lba = ArrayConfig::META_SSD_START_LBA;
//    EXPECT_EQ(lba, psa.lba);
//
//    //USER_DATA
//    lsa = { .stripeId = 0, .offset = 0 };
//    psa = sysArray->Translate(PartitionType::USER_DATA,lsa);
//    EXPECT_EQ(*(uBlockSet.data.begin()+1), psa.dev);
//    const PartitionLogicalSize* msSize =
//        sysArray->GetSizeInfo(PartitionType::META_SSD);
//    lba += ArrayConfig::BLOCKS_PER_CHUNK * msSize->totalStripes
//        * ArrayConfig::SECTORS_PER_BLOCK;
//    EXPECT_EQ(lba, psa.lba);
//}
//
// TEST_F(ArrayFixture, ConvertMetaNVM)
//{
//    uint32_t mnBlkCnt = 1;
//    LogicalBlkAddr lsa = { .stripeId = 0, .offset = 0};
//    LogicalWriteEntry logicalEntry;
//    logicalEntry.addr = lsa;
//    logicalEntry.blkCnt = mnBlkCnt;
//    Buffer buffer = AllocBuffer(mnBlkCnt);
//    logicalEntry.buffers.push_back(buffer);
//
//    list<PhysicalWriteEntry> physicalEntries =
//        sysArray->Convert(PartitionType::META_NVM, logicalEntry);
//    EXPECT_EQ(1, physicalEntries.size());
//    PhysicalWriteEntry physicalEntry = *physicalEntries.begin();
//    EXPECT_EQ(uBlockSet.nvm, physicalEntry.addr.dev);
//    EXPECT_EQ(0, physicalEntry.addr.lba);
//    EXPECT_EQ(1, physicalEntry.buffers.size());
//    EXPECT_EQ(mnBlkCnt, physicalEntry.blkCnt);
//    buffer = *physicalEntry.buffers.begin();
//    EXPECT_EQ(mnBlkCnt, buffer.blkCnt);
//    void* memInput = malloc(4096);
//    memset(memInput, 1, 4096);
//    EXPECT_EQ(0, memcmp(buffer.mem, memInput, 4096));
//    free(buffer.mem);
//    free(memInput);
//}
//
// TEST_F(ArrayFixture, ConvertWriteBuffer)
//{
//    uint32_t wbBlkCnt = 2;
//    uint32_t wbMemSize = 8192;
//    LogicalBlkAddr lsa = { .stripeId = 0, .offset = 0};
//    LogicalWriteEntry logicalEntry;
//    logicalEntry.addr = lsa;
//    logicalEntry.blkCnt = wbBlkCnt;
//    Buffer buffer = AllocBuffer(wbBlkCnt);
//    logicalEntry.buffers.push_back(buffer);
//
//    list<PhysicalWriteEntry> physicalEntries =
//        sysArray->Convert(PartitionType::WRITE_BUFFER, logicalEntry);
//    EXPECT_EQ(1, physicalEntries.size());
//    PhysicalWriteEntry physicalEntry = *physicalEntries.begin();
//    EXPECT_EQ(uBlockSet.nvm, physicalEntry.addr.dev);
//    const PartitionLogicalSize* mnSize =
//        sysArray->GetSizeInfo(PartitionType::META_NVM);
//    uint64_t lba = mnSize->blksPerStripe * mnSize->totalStripes
//        * ArrayConfig::SECTORS_PER_BLOCK;
//    EXPECT_EQ(lba, physicalEntry.addr.lba);
//    EXPECT_EQ(1, physicalEntry.buffers.size());
//    EXPECT_EQ(wbBlkCnt, physicalEntry.blkCnt);
//    buffer = *physicalEntry.buffers.begin();
//    EXPECT_EQ(wbBlkCnt, buffer.blkCnt);
//    void* memInput = malloc(wbMemSize);
//    memset(memInput, 1, wbMemSize);
//    EXPECT_EQ(0, memcmp(buffer.mem, memInput, wbMemSize));
//    free(buffer.mem);
//    free(memInput);
//}
//
// TEST_F(ArrayFixture, ConvertMetaSSD)
//{
//    uint32_t msBlkCnt = 1;
//    uint32_t msMemSize = 4096;
//    LogicalBlkAddr lsa = { .stripeId = 0, .offset = 0};
//    LogicalWriteEntry logicalEntry;
//    logicalEntry.addr = lsa;
//    logicalEntry.blkCnt = msBlkCnt;
//    Buffer buffer = AllocBuffer(msBlkCnt);
//    logicalEntry.buffers.push_back(buffer);
//
//    list<PhysicalWriteEntry> physicalEntries =
//        sysArray->Convert(PartitionType::META_SSD, logicalEntry);
//    EXPECT_EQ(2, physicalEntries.size());
//
//    int i = 0;
//    uint32_t lba = ArrayConfig::META_SSD_START_LBA;
//    void* memInput = malloc(msMemSize);
//    memset(memInput, 1, msMemSize);
//    for(PhysicalWriteEntry& physicalEntry: physicalEntries)
//    {
//        EXPECT_EQ(*(uBlockSet.data.begin()+i), physicalEntry.addr.dev);
//        EXPECT_EQ(lba, physicalEntry.addr.lba);
//        EXPECT_EQ(1, physicalEntry.buffers.size());
//        EXPECT_EQ(msBlkCnt, physicalEntry.blkCnt);
//        buffer = *physicalEntry.buffers.begin();
//        EXPECT_EQ(msBlkCnt, buffer.blkCnt);
//        EXPECT_EQ(0, memcmp(buffer.mem, memInput, msMemSize));
//        i++;
//    }
//    free(buffer.mem);
//    free(memInput);
//}
//
// TEST_F(ArrayFixture, ConvertUserData)
//{
//    uint32_t chunkCnt = 2;
//    uint32_t udBlksPerChunk = 64;
//    uint32_t udMemSize = 4096 * 64;
//    LogicalBlkAddr lsa = { .stripeId = 0, .offset = 0};
//    LogicalWriteEntry logicalEntry;
//    logicalEntry.addr = lsa;
//    logicalEntry.blkCnt = chunkCnt * udBlksPerChunk;
//    for (uint32_t i = 0; i < chunkCnt; i++)
//    {
//        Buffer buffer = AllocBuffer(udBlksPerChunk);
//        logicalEntry.buffers.push_back(buffer);
//    }
//
//    list<PhysicalWriteEntry> physicalEntries =
//        sysArray->Convert(PartitionType::USER_DATA, logicalEntry);
//    EXPECT_EQ(3, physicalEntries.size());
//
//    const PartitionLogicalSize* msSize =
//        sysArray->GetSizeInfo(PartitionType::META_SSD);
//    uint32_t lba = ArrayConfig::META_SSD_START_LBA +
//        ArrayConfig::BLOCKS_PER_CHUNK * msSize->totalStripes
//        * ArrayConfig::SECTORS_PER_BLOCK;
//
//    uint32_t i = 0;
//    void* memInput = malloc(udMemSize);
//    memset(memInput, 1, udMemSize);
//    void* memParity = malloc(udMemSize);
//    memset(memParity, 1^1, udMemSize);
//
//    for(PhysicalWriteEntry& physicalEntry: physicalEntries)
//    {
//        EXPECT_EQ(*(uBlockSet.data.begin()+i), physicalEntry.addr.dev);
//        EXPECT_EQ(lba, physicalEntry.addr.lba);
//        EXPECT_EQ(1, physicalEntry.buffers.size());
//        EXPECT_EQ(udBlksPerChunk, physicalEntry.blkCnt);
//        Buffer buffer = *physicalEntry.buffers.begin();
//        EXPECT_EQ(udBlksPerChunk, buffer.blkCnt);
//        if(i == lsa.stripeId % chunkCnt)
//        {
//            EXPECT_EQ(0, memcmp(buffer.mem, memParity, udMemSize));
//        }
//        else
//        {
//            EXPECT_EQ(0, memcmp(buffer.mem, memInput, udMemSize));
//        }
//        i++;
//    }
//
//    for(Buffer& buffer: logicalEntry.buffers)
//    {
//        free(buffer.mem);
//    }
//    free(memInput);
//    free(memParity);
//}

} //namespace
