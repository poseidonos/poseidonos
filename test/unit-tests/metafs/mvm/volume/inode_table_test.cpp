#include "src/metafs/mvm/volume/inode_table.h"

#include <gtest/gtest.h>

namespace pos
{

static void CheckAndAssertIfNotEqual(MetaFileInode& left, MetaFileInode& right)
{   
    // table1 data and newTable data must be same.
    EXPECT_EQ(left.data.basic.field.age, right.data.basic.field.age);
    EXPECT_EQ(left.data.basic.field.ctime, right.data.basic.field.ctime);
    EXPECT_EQ(left.data.basic.field.fd, right.data.basic.field.fd);
    EXPECT_EQ(left.data.basic.field.inUse, right.data.basic.field.inUse);
    EXPECT_EQ(left.data.basic.field.referenceCnt, right.data.basic.field.referenceCnt);
    EXPECT_EQ(left.data.basic.field.fileName.ToString(), right.data.basic.field.fileName.ToString());
    EXPECT_EQ(left.data.basic.field.fileByteSize, right.data.basic.field.fileByteSize);
    EXPECT_EQ(left.data.basic.field.dataChunkSize, right.data.basic.field.dataChunkSize);
    EXPECT_EQ(left.data.basic.field.ioAttribute.ioSpecfic.integrity, right.data.basic.field.ioAttribute.ioSpecfic.integrity);
    EXPECT_EQ(left.data.basic.field.ioAttribute.ioSpecfic.type, right.data.basic.field.ioAttribute.ioSpecfic.type);
    EXPECT_EQ(left.data.basic.field.ioAttribute.media, right.data.basic.field.ioAttribute.media);
    EXPECT_EQ(left.data.basic.field.indexInInodeTable, right.data.basic.field.indexInInodeTable);
    EXPECT_EQ(left.data.basic.field.versionSignature, right.data.basic.field.versionSignature);
    EXPECT_EQ(left.data.basic.field.version, right.data.basic.field.version);
    EXPECT_EQ(left.data.basic.field.pagemapCnt, right.data.basic.field.pagemapCnt);
    for(int j = 0; j < left.data.basic.field.pagemapCnt; ++j)
    {
        EXPECT_EQ(left.data.basic.field.pagemap[j].GetStartLpn(), right.data.basic.field.pagemap[j].GetStartLpn());
        EXPECT_EQ(left.data.basic.field.pagemap[j].GetCount(), right.data.basic.field.pagemap[j].GetCount());
    }
    EXPECT_EQ(left.data.basic.field.pagemapCnt, right.data.basic.field.pagemapCnt);
    EXPECT_EQ(left.data.basic.field.pagemapCnt, right.data.basic.field.pagemapCnt);
}

TEST(InodeTable, CreateTable)
{
    InodeTable table(MetaVolumeType::SsdVolume, 0);

    table.Create(0);

    EXPECT_EQ(table.GetFileDescriptor(0), 0);
}

TEST(InodeTable, CheckInode)
{
    InodeTable table(MetaVolumeType::SsdVolume, 0);

    MetaFileInodeArray& array = table.GetInodeArray();

    array[0].data.basic.field.age = 1;
    array[0].data.basic.field.ctime = 2;
    array[0].data.basic.field.fd = 3;
    array[0].data.basic.field.inUse = true;

    MetaFileInode& inode = table.GetInode(0);

    EXPECT_EQ(array[0].data.basic.field.age, inode.data.basic.field.age);
    EXPECT_EQ(array[0].data.basic.field.ctime, inode.data.basic.field.ctime);
    EXPECT_EQ(array[0].data.basic.field.fd, inode.data.basic.field.fd);
    EXPECT_EQ(array[0].data.basic.field.inUse, inode.data.basic.field.inUse);
}


TEST(InodeTable, ToBytesByIndex_testIfDeserializedObjContainsOriginalData)
{
    InodeTable table1(MetaVolumeType::SsdVolume, 0);
    InodeTable newTable(MetaVolumeType::SsdVolume, 0);

    MetaFileInodeArray& array = table1.GetInodeArray();
    
    // Assume InodeTableEntryNumber is 20
    int InodeTableEntryNumber = 20;

    // Set InodeTable Index to specific value less than InodeTableEntryNumber
    int entryIdx = 10; 

    // Set InodeTableContent Values
    array[entryIdx].data.basic.field.age = 1;
    array[entryIdx].data.basic.field.ctime = 2;
    array[entryIdx].data.basic.field.fd = 3;
    array[entryIdx].data.basic.field.inUse = true;
    array[entryIdx].data.basic.field.referenceCnt = 5;
    array[entryIdx].data.basic.field.fileName = "tmpName";
    array[entryIdx].data.basic.field.fileByteSize = 4096;
    array[entryIdx].data.basic.field.dataChunkSize = 128;
    array[entryIdx].data.basic.field.ioAttribute.ioSpecfic.integrity = (MetaFileIntegrityType::Lvl0_Disable);
    array[entryIdx].data.basic.field.ioAttribute.ioSpecfic.type = (MetaFileType::General);
    array[entryIdx].data.basic.field.ioAttribute.media = (MetaStorageType::SSD);
    array[entryIdx].data.basic.field.indexInInodeTable = 20;
    array[entryIdx].data.basic.field.versionSignature = 1929;
    array[entryIdx].data.basic.field.version = 3;
    array[entryIdx].data.basic.field.pagemapCnt = 10;

    for(int j = 0; j < array[entryIdx].data.basic.field.pagemapCnt; ++j)
    {
        array[entryIdx].data.basic.field.pagemap[j].SetStartLpn(j);
        array[entryIdx].data.basic.field.pagemap[j].SetCount(j);
    }

    array[entryIdx].data.basic.field.ageCopy = 1;
    array[entryIdx].data.basic.field.ctimeCopy = 2;
    

    int protoBufSize = InodeTableContentOnSsdSize * InodeTableEntryNumber; 
    char ioBuffer[protoBufSize];

    // Set Buffer Empty
    memset(ioBuffer, 0, protoBufSize);

    // Write table1 contents to Buffer
    table1.GetContent()->ToBytesByIndex(ioBuffer, entryIdx);

    // Write Buffer Contents to newTable
    newTable.GetContent()->FromBytesByIndex(ioBuffer, entryIdx);

    // table1 data and newTable data must be same.
    CheckAndAssertIfNotEqual(table1.GetContent()->entries[entryIdx], newTable.GetContent()->entries[entryIdx]);
}
} // namespace pos
