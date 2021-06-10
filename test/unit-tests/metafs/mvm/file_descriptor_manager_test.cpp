#include "src/metafs/mvm/file_descriptor_manager.h"

#include <unordered_map>
#include <string>
#include <gtest/gtest.h>

namespace pos
{
TEST(FileDescriptorManager, Create)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::map<FileDescriptorType, FileDescriptorType>* freeFDMap =
        new std::map<FileDescriptorType, FileDescriptorType>();

    FileDescriptorManager* fileMgr = new FileDescriptorManager(lookupMap, freeFDMap);

    EXPECT_EQ(&(fileMgr->GetFDLookupMap()), lookupMap);
    EXPECT_EQ(&(fileMgr->GetFreeFDMap()), freeFDMap);

    delete fileMgr;
}

TEST(FileDescriptorManager, Reset)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::map<FileDescriptorType, FileDescriptorType>* freeFDMap =
        new std::map<FileDescriptorType, FileDescriptorType>();

    FileDescriptorManager* fileMgr = new FileDescriptorManager(lookupMap, freeFDMap);

    std::string fileName = "TESTFILE";
    StringHashType fileKey = 123456;

    fileMgr->Free(0);
    fileMgr->InsertFileDescLookupHash(fileKey, 1);

    EXPECT_NE(lookupMap->size(), 0);
    EXPECT_NE(freeFDMap->size(), 0);

    fileMgr->Reset();

    EXPECT_EQ(lookupMap->size(), 0);
    EXPECT_EQ(freeFDMap->size(), 0);

    delete fileMgr;
}

TEST(FileDescriptorManager, CheckFreeMap0)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::map<FileDescriptorType, FileDescriptorType>* freeFDMap =
        new std::map<FileDescriptorType, FileDescriptorType>();

    FileDescriptorManager* fileMgr = new FileDescriptorManager(lookupMap, freeFDMap);
    uint32_t maxFile = MetaFsConfig::MAX_META_FILE_NUM_SUPPORT;

    fileMgr->AddAllFDsInFreeFDMap();

    EXPECT_EQ(freeFDMap->size(), maxFile);

    for (uint32_t fd = 0; fd < maxFile; fd++)
    {
        EXPECT_EQ(fileMgr->Alloc(), fd);
    }

    EXPECT_EQ(freeFDMap->size(), 0);

    delete fileMgr;
}

TEST(FileDescriptorManager, CheckFreeMap1)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::map<FileDescriptorType, FileDescriptorType>* freeFDMap =
        new std::map<FileDescriptorType, FileDescriptorType>();

    FileDescriptorManager* fileMgr = new FileDescriptorManager(lookupMap, freeFDMap);
    uint32_t maxFile = MetaFsConfig::MAX_META_FILE_NUM_SUPPORT;

    for (uint32_t fd = 0; fd < maxFile; fd++)
    {
        fileMgr->Free(fd);
    }

    EXPECT_EQ(freeFDMap->size(), maxFile);

    for (uint32_t fd = 0; fd < maxFile; fd++)
    {
        EXPECT_EQ(fileMgr->Alloc(), fd);
    }

    EXPECT_EQ(freeFDMap->size(), 0);

    delete fileMgr;
}

TEST(FileDescriptorManager, FindFDByName)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::map<FileDescriptorType, FileDescriptorType>* freeFDMap =
        new std::map<FileDescriptorType, FileDescriptorType>();

    FileDescriptorManager* fileMgr = new FileDescriptorManager(lookupMap, freeFDMap);

    std::string fileName = "TESTFILE";
    StringHashType fileKey = 123456;
    FileDescriptorType fd = 10;

    fileMgr->InsertFileDescLookupHash(fileKey, fd);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_EQ(fileMgr->FindFDByName(fileKey), fd);

    delete fileMgr;
}

TEST(FileDescriptorManager, CheckCreateFile)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::map<FileDescriptorType, FileDescriptorType>* freeFDMap =
        new std::map<FileDescriptorType, FileDescriptorType>();

    FileDescriptorManager* fileMgr = new FileDescriptorManager(lookupMap, freeFDMap);

    std::string fileName = "TESTFILE";
    StringHashType fileKey = 123456;
    FileDescriptorType fd = 10;

    fileMgr->InsertFileDescLookupHash(fileKey, fd);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_TRUE(fileMgr->IsGivenFileCreated(fileKey));

    delete fileMgr;
}

} // namespace pos
