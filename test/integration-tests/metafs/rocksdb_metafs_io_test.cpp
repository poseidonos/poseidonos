/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <algorithm>
#include <atomic>
#include <cstring>

#include "src/metafs/common/metafs_stopwatch.h"
#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "test/integration-tests/metafs/lib/rocksdb_test_metafs.h"

using namespace std;

namespace pos
{
class RocksDbMetaFsIoTest : public RocksDbTestMetaFs
{
public:
    RocksDbMetaFsIoTest(void)
    : RocksDbTestMetaFs()
    {
        // We intentionally set the array id to 0 in this fixture, just for testing purposes.
        arrayId = 0;
    }
    virtual ~RocksDbMetaFsIoTest(void)
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
MakeSequentialDataPattern(void)
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

TEST_F(RocksDbMetaFsIoTest, testIfTheSameDataCanBeRetrievedByReadAfterWritingData_InRotation)
{
    memset(writeBuf, 0, BYTE_4K);

    // // write -> read -> write -> read -> ..
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        AsyncMetaFileIoCtx* writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, i * BYTE_4K, BYTE_4K, writeBuf);
        int result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
        ASSERT_EQ(result, EID(SUCCESS)) << "write fail code: " << (int)result;

        AsyncMetaFileIoCtx* readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, i * BYTE_4K, BYTE_4K, readBuf);
        result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
        ASSERT_EQ(result, EID(SUCCESS)) << "read fail code: " << (int)result;

        ASSERT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;

        delete writeReq;
        delete readReq;
    }
}

TEST_F(RocksDbMetaFsIoTest, testIfTheSameDataCanBeRetrievedByReadAfterWritingData_FirstWriteLastRead)
{
    // write
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        AsyncMetaFileIoCtx* writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, i * BYTE_4K, BYTE_4K, writeBuf);
        int result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
        ASSERT_EQ(result, EID(SUCCESS)) << "write fail code: " << (int)result;
        delete writeReq;
    }

    // read and verify
    for (int i = 0; i < COUNT_OF_META_LPN_FOR_SSD; ++i)
    {
        *(int*)writeBuf = i + 1;

        AsyncMetaFileIoCtx* readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, i * BYTE_4K, BYTE_4K, readBuf);
        int result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
        ASSERT_EQ(result, EID(SUCCESS)) << "read fail code: " << (int)result;
        ASSERT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
        delete readReq;
    }
}

TEST_F(RocksDbMetaFsIoTest, testIfMetaFsCanRejectTheRequestsDueToOutOfRange)
{
    AsyncMetaFileIoCtx* writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, (COUNT_OF_META_LPN_FOR_SSD + 1) * BYTE_4K, BYTE_4K, writeBuf);
    int result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
    ASSERT_EQ(result, ERRID(ROCKSDB_MFS_ASYNCIO_OFFSET_ERROR));
    delete writeReq;

    AsyncMetaFileIoCtx* readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, (COUNT_OF_META_LPN_FOR_SSD + 1) * BYTE_4K, BYTE_4K, readBuf);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
    EXPECT_EQ(result, ERRID(ROCKSDB_MFS_ASYNCIO_OFFSET_ERROR));
    delete readReq;
}

TEST_F(RocksDbMetaFsIoTest, testIfPartialSyncIoCanBeWork)
{
    std::generate_n(writeBuf, BYTE_4K, MakeSequentialDataPattern);

    AsyncMetaFileIoCtx* writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, 0, 4, writeBuf);
    int result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
    ASSERT_EQ(result, EID(SUCCESS));
    delete writeReq;

    writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, 4, 128, writeBuf + 4);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
    ASSERT_EQ(result, EID(SUCCESS));
    delete writeReq;

    writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, 132, 8, writeBuf + 132);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
    ASSERT_EQ(result, EID(SUCCESS));
    delete writeReq;

    writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, 140, BYTE_4K - 140, writeBuf + 140);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
    ASSERT_EQ(result, EID(SUCCESS));
    delete writeReq;

    memset(readBuf, 0, BYTE_4K);

    AsyncMetaFileIoCtx* readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, 0, 4, readBuf);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
    EXPECT_EQ(result, EID(SUCCESS));
    delete readReq;

    readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, 4, 128, readBuf + 4);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
    EXPECT_EQ(result, EID(SUCCESS));
    delete readReq;

    readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, 132, 8, readBuf + 132);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
    EXPECT_EQ(result, EID(SUCCESS));
    delete readReq;

    readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, 140, BYTE_4K - 140, readBuf + 140);
    result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
    EXPECT_EQ(result, EID(SUCCESS));
    delete readReq;

    EXPECT_TRUE(std::equal(readBuf, readBuf + BYTE_4K, writeBuf)) << *(int*)readBuf;
}

TEST_F(RocksDbMetaFsIoTest, testMetaFsPerformance_SyncWrite)
{
    const size_t TEST_SIZE = BYTE_4K * COUNT_OF_META_LPN_FOR_SSD;
    MetaFsStopwatch<PerformanceStage> stopWatch;

    stopWatch.StoreTimestamp();
    for (int i = 0; i < TEST_SIZE / BYTE_4K; ++i)
    {
        AsyncMetaFileIoCtx* writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, i * BYTE_4K, BYTE_4K, writeBuf);
        int result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
        ASSERT_EQ(result, EID(SUCCESS)) << "iteration: " << i << ", write fail code: " << (int)result;
        delete writeReq;
    }
    stopWatch.StoreTimestamp();

    std::cout << "Write Size: " << (double)TEST_SIZE / 1024 / 1024 << " MB" << std::endl;
    std::cout << "ElapsedTime: " << stopWatch.GetElapsedInMilli().count() << " ms" << std::endl;
    std::cout << "Performance: " << (double)((double)TEST_SIZE / stopWatch.GetElapsedInMilli().count()) / 1024 / 1024 * 1000 << " MB/s" << std::endl;
}

TEST_F(RocksDbMetaFsIoTest, testMetaFsPerformance_SyncRead)
{
    const size_t TEST_SIZE = BYTE_4K * COUNT_OF_META_LPN_FOR_SSD;
    MetaFsStopwatch<PerformanceStage> stopWatch;

    // Add Dummy Write
    for (int i = 0; i < TEST_SIZE / BYTE_4K; ++i)
    {
        AsyncMetaFileIoCtx* writeReq = CreateRequest(MetaFsIoOpcode::Write, arrayId, i * BYTE_4K, BYTE_4K, writeBuf);
        int result = rocksDBMetaFsList[arrayId]->AsyncIO(writeReq);
        ASSERT_EQ(result, EID(SUCCESS)) << "iteration: " << i << ", write fail code: " << (int)result;
        delete writeReq;
    }

    // Read Dummy Write
    stopWatch.StoreTimestamp();
    for (int i = 0; i < TEST_SIZE / BYTE_4K; ++i)
    {
        AsyncMetaFileIoCtx* readReq = CreateRequest(MetaFsIoOpcode::Read, arrayId, i * BYTE_4K, BYTE_4K, readBuf);
        int result = rocksDBMetaFsList[arrayId]->AsyncIO(readReq);
        EXPECT_EQ(result, EID(SUCCESS)) << "iteration: " << i << ", read fail code: " << (int)result;
        delete readReq;
    }
    stopWatch.StoreTimestamp();

    std::cout << "Read Size: " << (double)TEST_SIZE / 1024 / 1024 << " MB" << std::endl;
    std::cout << "ElapsedTime: " << stopWatch.GetElapsedInMilli().count() << " ms" << std::endl;
    std::cout << "Performance: " << (double)((double)TEST_SIZE / stopWatch.GetElapsedInMilli().count()) / 1024 / 1024 * 1000 << " MB/s" << std::endl;
}
} // namespace pos
