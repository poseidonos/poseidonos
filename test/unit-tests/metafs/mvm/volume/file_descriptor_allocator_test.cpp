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

#include "src/metafs/mvm/volume/file_descriptor_allocator.h"

#include <set>
#include <unordered_map>
#include <string>
#include <gtest/gtest.h>

namespace pos
{
TEST(FileDescriptorAllocator, Create)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);

    EXPECT_EQ(fdAllocator->GetLookupMap(), lookupMap);
    EXPECT_EQ(fdAllocator->GetFreeMap(), freeMap);

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, Reset)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);
    uint32_t maxFile = fdAllocator->GetMaxFileCount();

    EXPECT_EQ(lookupMap->size(), 0);
    EXPECT_EQ(freeMap->size(), maxFile);

    std::string fileName = "TESTFILE";
    StringHashType fileKey = 123456;

    fdAllocator->Alloc(fileName);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_EQ(freeMap->size(), maxFile - 1);

    fdAllocator->Reset();

    EXPECT_EQ(lookupMap->size(), 0);
    EXPECT_EQ(freeMap->size(), maxFile);

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, CheckFreeMap)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);
    uint32_t maxFile = fdAllocator->GetMaxFileCount();
    std::string fileName = "TESTFILE";

    EXPECT_EQ(freeMap->size(), maxFile);

    for (uint32_t fd = 0; fd < maxFile; fd++)
    {
        EXPECT_EQ(fdAllocator->Alloc(fileName), fd);
    }

    EXPECT_EQ(freeMap->size(), 0);

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, FindFdByName)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);

    std::string fileName = "TESTFILE";

    FileDescriptorType fd = fdAllocator->Alloc(fileName);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_EQ(fdAllocator->FindFdByName(fileName), fd);

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, FindFdByHashKey)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);

    std::string fileName = "TESTFILE";
    StringHashType fileKey = 123456;

    FileDescriptorType fd = fdAllocator->Alloc(fileKey);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_EQ(fdAllocator->FindFdByHashKey(fileKey), fd);

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, CheckCreateFile)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);

    StringHashType fileKey = 123456;
    FileDescriptorType fd = 10;

    fdAllocator->Alloc(fileKey);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_TRUE(fdAllocator->IsGivenFileCreated(fileKey));

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, FreeByName)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);
    std::string fileName = "TESTFILE";

    FileDescriptorType fd = fdAllocator->Alloc(fileName);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_TRUE(fdAllocator->IsGivenFileCreated(fileName));

    fdAllocator->Free(fileName, fd);

    EXPECT_FALSE(fdAllocator->IsGivenFileCreated(fileName));

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, FreeByHashKey)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);
    StringHashType fileKey = 123456;

    FileDescriptorType fd = fdAllocator->Alloc(fileKey);

    EXPECT_EQ(lookupMap->size(), 1);
    EXPECT_TRUE(fdAllocator->IsGivenFileCreated(fileKey));

    fdAllocator->Free(fileKey, fd);

    EXPECT_FALSE(fdAllocator->IsGivenFileCreated(fileKey));

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, UpdateFreeMap)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);
    uint32_t maxFile = fdAllocator->GetMaxFileCount();

    EXPECT_EQ(freeMap->size(), maxFile);

    fdAllocator->UpdateFreeMap(0);

    EXPECT_EQ(freeMap->size(), maxFile - 1);

    for (FileDescriptorType fdesc = 1; fdesc < maxFile; fdesc++)
    {
        fdAllocator->UpdateFreeMap(fdesc);
    }

    EXPECT_EQ(freeMap->size(), 0);

    delete fdAllocator;
}

TEST(FileDescriptorAllocator, UpdateLookupMap)
{
    std::unordered_map<StringHashType, FileDescriptorType>* lookupMap =
        new std::unordered_map<StringHashType, FileDescriptorType>();
    std::set<FileDescriptorType>* freeMap = new std::set<FileDescriptorType>();

    FileDescriptorAllocator* fdAllocator = new FileDescriptorAllocator(lookupMap, freeMap);
    uint32_t maxFile = fdAllocator->GetMaxFileCount();
    StringHashType fileKey = 123456;

    EXPECT_EQ(lookupMap->size(), 0);

    fdAllocator->UpdateLookupMap(fileKey, 0);

    EXPECT_EQ(lookupMap->size(), 1);

    delete fdAllocator;
}

} // namespace pos
