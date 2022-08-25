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

#include <cstring>
#include <unordered_map>

#include "test/integration-tests/metafs/lib/test_metafs.h"
#include "test/integration-tests/metafs/lib/metafs_test_fixture.h"

using namespace std;

namespace pos
{
class MetaFsFunctionalTest : public MetaFsTestFixture
{
public:
    MetaFsFunctionalTest(void)
    : MetaFsTestFixture()
    {
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            files[arrayId].insert({MetaVolumeType::SsdVolume,
                {"TestFileSsd", BYTE_4K * COUNT_OF_META_LPN_FOR_SSD, 0,
                    {MetaFileIntegrityType::Default, MetaFileType::General}}});
            files[arrayId].insert({MetaVolumeType::NvRamVolume,
                {"TestFileNvm", BYTE_4K * COUNT_OF_META_LPN_FOR_NVM, 0,
                    {MetaFileIntegrityType::Default, MetaFileType::Journal}}});
            files[arrayId].insert({MetaVolumeType::JournalVolume,
                {"TestFileJournalSsd", BYTE_4K * COUNT_OF_META_LPN_FOR_JOURNAL_SSD, 0,
                    {MetaFileIntegrityType::Default, MetaFileType::Journal}}});
        }

        writeBuf = new char[BYTE_4K];
        readBuf = new char[BYTE_4K];

        memset(writeBuf, 0, BYTE_4K);
        memset(readBuf, 0, BYTE_4K);
    }
    virtual ~MetaFsFunctionalTest(void)
    {
        delete[] writeBuf;
        delete[] readBuf;
    }

    std::unordered_map<int, std::unordered_map<MetaVolumeType, FileInformation>> files;

protected:
    const size_t BYTE_4K = 4032;
    const size_t COUNT_OF_META_LPN_FOR_SSD = 1000;
    const size_t COUNT_OF_META_LPN_FOR_NVM = 100;
    const size_t COUNT_OF_META_LPN_FOR_JOURNAL_SSD = 200;

    char* writeBuf;
    char* readBuf;
};

TEST(MetaFsFunctionalTest, testIfMetaFsCanRepeatMountAndUnmount)
{
    const size_t REPEAT_COUNT = 30;
    MetaFsFunctionalTest metaFs;

    for (size_t count = 0; count < REPEAT_COUNT; ++count)
    {
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            // mount array
            ASSERT_EQ(0, metaFs.GetMetaFs(arrayId)->Init());

            // meta file
            POS_EVENT_ID rc_mgmt;

            for (auto& info : metaFs.files[arrayId])
            {
                rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                    info.second.fileSize, info.second.prop, info.first);
                ASSERT_EQ(rc_mgmt, EID(SUCCESS));
                ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), EID(SUCCESS));
                ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), EID(SUCCESS));
            }

            // unmount array
            metaFs.GetMetaFs(arrayId)->Dispose();
        }
    }
}

TEST(MetaFsFunctionalTest, testIfMetaFilesCanBeCreated)
{
    const size_t REPEAT_COUNT = 30;
    MetaFsFunctionalTest metaFs;

    for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
    {
        // mount array
        ASSERT_EQ(0, metaFs.GetMetaFs(arrayId)->Init());

        // meta file
        POS_EVENT_ID rc_mgmt;

        for (auto& info : metaFs.files[arrayId])
        {
            rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                info.second.fileSize, info.second.prop, info.first);
            ASSERT_EQ(rc_mgmt, EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), EID(SUCCESS));
        }

        // unmount array
        metaFs.GetMetaFs(arrayId)->Dispose();
    }
}

TEST(MetaFsFunctionalTest, testIfMetaFilesCannotBeCreatedDueToDuplicatedFileName)
{
    MetaFsFunctionalTest metaFs;

    for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
    {
        // mount array
        ASSERT_EQ(0, metaFs.GetMetaFs(arrayId)->Init());

        // meta file
        POS_EVENT_ID rc_mgmt;

        for (auto& info : metaFs.files[arrayId])
        {
            rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                info.second.fileSize, info.second.prop, info.first);
            ASSERT_EQ(rc_mgmt, EID(SUCCESS));
            rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                info.second.fileSize, info.second.prop, info.first);
            // then
            EXPECT_NE(rc_mgmt, EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), EID(SUCCESS));
        }

        // unmount array
        metaFs.GetMetaFs(arrayId)->Dispose();
    }
}

TEST(MetaFsFunctionalTest, testIfMetaFilesWillNotOpenTwice)
{
    MetaFsFunctionalTest metaFs;

    for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
    {
        // mount array
        ASSERT_EQ(0, metaFs.GetMetaFs(arrayId)->Init());

        // meta file
        POS_EVENT_ID rc_mgmt;

        for (auto& info : metaFs.files[arrayId])
        {
            rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                info.second.fileSize, info.second.prop, info.first);
            ASSERT_EQ(rc_mgmt, EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), EID(SUCCESS));
            // then
            EXPECT_NE(metaFs.GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), EID(SUCCESS));
        }

        // unmount array
        metaFs.GetMetaFs(arrayId)->Dispose();
    }
}

TEST(MetaFsFunctionalTest, testIfMetaFilesWillNotCloseTwice)
{
    MetaFsFunctionalTest metaFs;

    for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
    {
        // mount array
        ASSERT_EQ(0, metaFs.GetMetaFs(arrayId)->Init());

        // meta file
        POS_EVENT_ID rc_mgmt;

        for (auto& info : metaFs.files[arrayId])
        {
            rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                info.second.fileSize, info.second.prop, info.first);
            ASSERT_EQ(rc_mgmt, EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), EID(SUCCESS));
            ASSERT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), EID(SUCCESS));
            // then
            EXPECT_NE(metaFs.GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), EID(SUCCESS));
        }

        // unmount array
        metaFs.GetMetaFs(arrayId)->Dispose();
    }
}

TEST(MetaFsFunctionalTest, testIfMetaFsDeliversTheChunkSize)
{
    // given
    // We intentionally set the array id to 0 in this fixture, just for testing purposes.
    const int arrayId = 0;
    const FileDescriptorType fd = 0;
    const MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    MetaFsFunctionalTest metaFs;
    uint64_t chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    POS_EVENT_ID rc_mgmt;

    // when
    ASSERT_EQ(0, metaFs.GetMetaFs(arrayId)->Init());
    rc_mgmt = metaFs.GetMetaFs(arrayId)->ctrl->Create(metaFs.files[arrayId][volumeType].fileName,
                metaFs.files[arrayId][volumeType].fileSize, metaFs.files[arrayId][volumeType].prop, volumeType);
    ASSERT_EQ(rc_mgmt, EID(SUCCESS));

    // then
    EXPECT_EQ(metaFs.GetMetaFs(arrayId)->ctrl->GetAlignedFileIOSize(metaFs.files[arrayId][volumeType].fd, volumeType), chunkSize);

    metaFs.GetMetaFs(arrayId)->Dispose();
}
} // namespace pos
