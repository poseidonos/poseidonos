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

#include <atomic>
#include <algorithm>
#include <cstring>

#include "src/metafs/common/metafs_stopwatch.h"
#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "test/integration-tests/metafs/lib/test_metafs.h"

using namespace std;

namespace pos
{
class MetaFsIoTest : public TestMetaFs
{
public:
    MetaFsIoTest(void)
    : TestMetaFs()
    {
        // We intentionally set the array id to 0 in this fixture, just for testing purposes.
        arrayId = 0;
    }
    virtual ~MetaFsIoTest(void)
    {
    }
    void DoneCallback(void* data)
    {
        ioDone = true;
    }

protected:
    int arrayId;
    std::atomic<bool> ioDone;
};

int
GenerateSequentialDataPattern(void)
{
    static int i = 0;
    return ++i;
}

enum PerformanceStage
{
    Start,
    Done,
    Count
};


TEST_F(MetaFsIoTest, testIfTheSameDataCanBeRetrievedByReadAfterWritingData_InRotation)
{
    memset(writeBuf, 0, BYTE_4K);

    // write -> read -> write -> read -> ..
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        POS_EVENT_ID result = GetMetaFs(arrayId)->io->Write(files[arrayId][MetaVolumeType::SsdVolume].fd, i * BYTE_4K, BYTE_4K, writeBuf);
        ASSERT_EQ(result, EID(SUCCESS)) << "write fail code: " << (int)result;
        result = GetMetaFs(arrayId)->io->Read(files[arrayId][MetaVolumeType::SsdVolume].fd, i * BYTE_4K, BYTE_4K, readBuf);
        ASSERT_EQ(result, EID(SUCCESS)) << "read fail code: " << (int)result;

        ASSERT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
    }
}

TEST_F(MetaFsIoTest, testIfTheSameDataCanBeRetrievedByReadAfterWritingData_FirstWriteLastRead)
{
    // write
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        POS_EVENT_ID result = GetMetaFs(arrayId)->io->Write(files[arrayId][MetaVolumeType::SsdVolume].fd, i * BYTE_4K, BYTE_4K, writeBuf);
        ASSERT_EQ(result, EID(SUCCESS)) << "write fail code: " << (int)result;
    }

    // read and verify
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        POS_EVENT_ID result = GetMetaFs(arrayId)->io->Read(files[arrayId][MetaVolumeType::SsdVolume].fd, i * BYTE_4K, BYTE_4K, readBuf);
        ASSERT_EQ(result, EID(SUCCESS)) << "read fail code: " << (int)result;
        ASSERT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
    }
}

TEST_F(MetaFsIoTest, testIfMetaFsCanRejectTheRequestsDueToOutOfRange)
{
    POS_EVENT_ID result = GetMetaFs(arrayId)->io->Write(files[arrayId][MetaVolumeType::SsdVolume].fd, (COUNT_OF_META_LPN_FOR_SSD + 1) * BYTE_4K, BYTE_4K, writeBuf);
    ASSERT_EQ(result, EID(MFS_INVALID_PARAMETER));

    result = GetMetaFs(arrayId)->io->Read(files[arrayId][MetaVolumeType::SsdVolume].fd, (COUNT_OF_META_LPN_FOR_SSD + 1) * BYTE_4K, BYTE_4K, readBuf);
    EXPECT_EQ(result, EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaFsIoTest, testIfPartialSyncIoCanBeWork)
{
    std::generate_n(writeBuf, BYTE_4K, GenerateSequentialDataPattern);
    POS_EVENT_ID rc = EID(SUCCESS);
    FileDescriptorType fd = files[arrayId][MetaVolumeType::SsdVolume].fd;

    rc = GetMetaFs(arrayId)->io->Write(fd, 0, 4, writeBuf, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));
    rc = GetMetaFs(arrayId)->io->Write(fd, 4, 128, writeBuf + 4, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));
    rc = GetMetaFs(arrayId)->io->Write(fd, 132, 8, writeBuf + 132, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));
    rc = GetMetaFs(arrayId)->io->Write(fd, 140, BYTE_4K - 140, writeBuf + 140, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));

    memset(readBuf, 0, BYTE_4K);
    rc = GetMetaFs(arrayId)->io->Read(fd, 0, 4, readBuf, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));
    rc = GetMetaFs(arrayId)->io->Read(fd, 4, 128, readBuf + 4, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));
    rc = GetMetaFs(arrayId)->io->Read(fd, 132, 8, readBuf + 132, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));
    rc = GetMetaFs(arrayId)->io->Read(fd, 140, BYTE_4K - 140, readBuf + 140, MetaStorageType::SSD);
    EXPECT_EQ(rc, EID(SUCCESS));

    EXPECT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
}

TEST_F(MetaFsIoTest, testMetaFsPerformance_SyncWrite)
{
    const size_t TEST_SIZE = BYTE_4K * COUNT_OF_META_LPN_FOR_SSD;
    MetaFsStopwatch<PerformanceStage> stopWatch;

    stopWatch.StoreTimestamp();
    for (int i = 0; i < TEST_SIZE / BYTE_4K; ++i)
    {
        POS_EVENT_ID result = GetMetaFs(arrayId)->io->Write(files[arrayId][MetaVolumeType::SsdVolume].fd, i * BYTE_4K, BYTE_4K, writeBuf);
        ASSERT_EQ(result, EID(SUCCESS)) << "iteration: " << i << ", write fail code: " << (int)result;
    }
    stopWatch.StoreTimestamp();

    std::cout << "Write Size: " << (double)TEST_SIZE / 1024 / 1024 << " MB" << std::endl;
    std::cout << "ElapsedTime: " << stopWatch.GetElapsedInMilli().count() << " ms" << std::endl;
    std::cout << "Performance: " << (double)((double)TEST_SIZE / stopWatch.GetElapsedInMilli().count()) / 1024 / 1024 * 1000 << " MB/s" << std::endl;
}

TEST_F(MetaFsIoTest, testMetaFsPerformance_AsyncWrite)
{
    const size_t TEST_SIZE = BYTE_4K * COUNT_OF_META_LPN_FOR_SSD;
    const FileDescriptorType fd = files[arrayId][MetaVolumeType::SsdVolume].fd;
    MetaFsStopwatch<PerformanceStage> stopWatch;
    char* buffer = new char[TEST_SIZE];

    ioDone = false;
    MetaFsAioCbCxt cxt(MetaFsIoOpcode::Write, fd, arrayId, 0, TEST_SIZE, buffer, [&](void* data) { ioDone = true; });

    stopWatch.StoreTimestamp();
    POS_EVENT_ID result = GetMetaFs(arrayId)->io->SubmitIO(&cxt);
    ASSERT_EQ(result, EID(SUCCESS));
    while (!ioDone)
    {
    }
    stopWatch.StoreTimestamp();

    std::cout << "Write Size: " << (double)TEST_SIZE / 1024 / 1024 << " MB" << std::endl;
    std::cout << "ElapsedTime: " << stopWatch.GetElapsedInMilli().count() << " ms" << std::endl;
    std::cout << "Performance: " << (double)((double)TEST_SIZE / stopWatch.GetElapsedInMilli().count()) / 1024 / 1024 * 1000 << " MB/s" << std::endl;

    delete[] buffer;
}

TEST_F(MetaFsIoTest, testMetaFsPerformance_SyncRead)
{
    const size_t TEST_SIZE = BYTE_4K * COUNT_OF_META_LPN_FOR_SSD;
    MetaFsStopwatch<PerformanceStage> stopWatch;

    stopWatch.StoreTimestamp();
    for (int i = 0; i < TEST_SIZE / BYTE_4K; ++i)
    {
        POS_EVENT_ID result = GetMetaFs(arrayId)->io->Read(files[arrayId][MetaVolumeType::SsdVolume].fd, i * BYTE_4K, BYTE_4K, readBuf);
        ASSERT_EQ(result, EID(SUCCESS)) << "iteration: " << i << ", write fail code: " << (int)result;
    }
    stopWatch.StoreTimestamp();

    std::cout << "Read Size: " << (double)TEST_SIZE / 1024 / 1024 << " MB" << std::endl;
    std::cout << "ElapsedTime: " << stopWatch.GetElapsedInMilli().count() << " ms" << std::endl;
    std::cout << "Performance: " << (double)((double)TEST_SIZE / stopWatch.GetElapsedInMilli().count()) / 1024 / 1024 * 1000 << " MB/s" << std::endl;
}

TEST_F(MetaFsIoTest, testMetaFsPerformance_AsyncRead)
{
    const size_t TEST_SIZE = BYTE_4K * COUNT_OF_META_LPN_FOR_SSD;
    const FileDescriptorType fd = files[arrayId][MetaVolumeType::SsdVolume].fd;
    MetaFsStopwatch<PerformanceStage> stopWatch;
    char* buffer = new char[TEST_SIZE];

    ioDone = false;
    MetaFsAioCbCxt cxt(MetaFsIoOpcode::Read, fd, arrayId, 0, TEST_SIZE, buffer, [&](void* data) { ioDone = true; });

    stopWatch.StoreTimestamp();
    POS_EVENT_ID result = GetMetaFs(arrayId)->io->SubmitIO(&cxt);
    ASSERT_EQ(result, EID(SUCCESS));
    while (!ioDone)
    {
    }
    stopWatch.StoreTimestamp();

    std::cout << "Read Size: " << (double)TEST_SIZE / 1024 / 1024 << " MB" << std::endl;
    std::cout << "ElapsedTime: " << stopWatch.GetElapsedInMilli().count() << " ms" << std::endl;
    std::cout << "Performance: " << (double)((double)TEST_SIZE / stopWatch.GetElapsedInMilli().count()) / 1024 / 1024 * 1000 << " MB/s" << std::endl;

    delete[] buffer;
}
} // namespace pos
