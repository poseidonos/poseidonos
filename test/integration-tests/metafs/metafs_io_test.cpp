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

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>

#include "test/integration-tests/metafs/metafs_test_fixture.h"

using namespace std;

namespace pos
{
struct FileInformation
{
    std::string fileName;
    uint64_t fileSize;
    int fd;
    MetaFilePropertySet prop;
};

class MetaFsIoTest : public MetaFsTestFixture, public ::testing::Test
{
public:
    MetaFsIoTest(void)
    : MetaFsTestFixture()
    {
        files.insert({StorageOpt::SSD, {"TestFileSsd", BYTE_4K * COUNT_OF_META_LPN_FOR_SSD, 0, {MetaFileAccessPattern::Default, MetaFileDominant::Default, MetaFileIntegrityType::Default}}});
        files.insert({StorageOpt::NVRAM, {"TestFileNvm", BYTE_4K * COUNT_OF_META_LPN_FOR_NVM, 0, {MetaFileAccessPattern::ByteIntensive, MetaFileDominant::Default, MetaFileIntegrityType::Default}}});

        writeBuf = new char[BYTE_4K];
        readBuf = new char[BYTE_4K];
    }

    virtual ~MetaFsIoTest(void)
    {
        delete[] writeBuf;
        delete[] readBuf;
    }

    virtual void SetUp(void)
    {
        // mount array
        ASSERT_EQ(0, metaFs->Init());

        // create meta file
        MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;

        for (auto& info : files)
        {
            rc_mgmt = metaFs->ctrl->Create(info.second.fileName,
                info.second.fileSize, info.second.prop, info.first);
            ASSERT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
            ASSERT_EQ(metaFs->ctrl->Open(info.second.fileName, info.second.fd, info.first), POS_EVENT_ID::SUCCESS);
        }
    }

    virtual void TearDown(void)
    {
        for (auto& info : files)
        {
            EXPECT_EQ(metaFs->ctrl->Close(info.second.fd, info.first), POS_EVENT_ID::SUCCESS);
        }

        // unmount array
        metaFs->Dispose();
    }

protected:
    std::unordered_map<StorageOpt, FileInformation> files;
    char* writeBuf;
    char* readBuf;

    const size_t BYTE_4K = 4032;
    const size_t COUNT_OF_META_LPN_FOR_SSD = 1000;
    const size_t COUNT_OF_META_LPN_FOR_NVM = 100;
};

TEST_F(MetaFsIoTest, testIfTheSameDataCanBeRetrievedByReadAfterWritingData_InRotation)
{
    memset(writeBuf, 0, BYTE_4K);

    // write -> read -> write -> read -> ..
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        POS_EVENT_ID result = metaFs->io->Write(files[StorageOpt::SSD].fd, i * BYTE_4K, BYTE_4K, writeBuf);
        ASSERT_EQ(result, POS_EVENT_ID::SUCCESS) << "write fail code: " << (int)result;
        result = metaFs->io->Read(files[StorageOpt::SSD].fd, i * BYTE_4K, BYTE_4K, readBuf);
        ASSERT_EQ(result, POS_EVENT_ID::SUCCESS) << "read fail code: " << (int)result;

        ASSERT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
    }
}

TEST_F(MetaFsIoTest, testIfTheSameDataCanBeRetrievedByReadAfterWritingData_FirstWriteLastRead)
{
    // write
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        POS_EVENT_ID result = metaFs->io->Write(files[StorageOpt::SSD].fd, i * BYTE_4K, BYTE_4K, writeBuf);
        ASSERT_EQ(result, POS_EVENT_ID::SUCCESS) << "write fail code: " << (int)result;
    }

    // read and verify
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        POS_EVENT_ID result = metaFs->io->Read(files[StorageOpt::SSD].fd, i * BYTE_4K, BYTE_4K, readBuf);
        ASSERT_EQ(result, POS_EVENT_ID::SUCCESS) << "read fail code: " << (int)result;
        ASSERT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
    }
}

TEST_F(MetaFsIoTest, testIfMetaFsCanRejectTheRequestsDueToOutOfRange)
{
    POS_EVENT_ID result = metaFs->io->Write(files[StorageOpt::SSD].fd, (COUNT_OF_META_LPN_FOR_SSD + 1) * BYTE_4K, BYTE_4K, writeBuf);
    ASSERT_EQ(result, POS_EVENT_ID::MFS_INVALID_PARAMETER);

    result = metaFs->io->Read(files[StorageOpt::SSD].fd, (COUNT_OF_META_LPN_FOR_SSD + 1) * BYTE_4K, BYTE_4K, readBuf);
    EXPECT_EQ(result, POS_EVENT_ID::MFS_INVALID_PARAMETER);
}
} // namespace pos
