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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "mfs_perf_test.h"

#include <algorithm>

#include "mfs.h"
#include "mfs_common.h"
#include "mfs_common_const.h"
#include "mfs_io_config.h"
#include "mfs_log.h"
#include "mfs_time.h"

TestClient client;
static std::atomic<bool> stopFlag;
static std::atomic<uint32_t> currQD;
static std::atomic<int> issuedAioCnt;
static std::atomic<int> completedAioCnt;
static std::vector<MetaFsAioCbCxt*> availableAioCb; // key: aiocb

void
ResetTestContext(void)
{
    stopFlag = false;
    currQD = 0;
    issuedAioCnt = 0;
    completedAioCnt = 0;
    availableAioCb.clear();
}

TEST_F(UtMetaFsPerf, FullPageWrite_Sync_QD1)
{
    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    size_t fileSize = 1024 * 1024 * 100;
    int fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    uint32_t dataChunkSize = metaFsMgr.util.GetAlignedFileIOSize(fd);
    EXPECT_NE(dataChunkSize, 0);

    FileSizeType targetOffset = 0;
    FileSizeType targetByteSize = dataChunkSize * 1; // single page read

    void* wBuf = calloc(1, targetByteSize);
    std::generate_n((uint8_t*)wBuf, targetByteSize, Sequential_data_pattern_gen);
    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;

    int it = fileSize / targetByteSize;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    EnableAIR();

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "T=[{}] Start full page write. Iteration=", GetTimestampUs(), it);
    while (it--)
    {
        rc_io = metaFsMgr.io.Write(fd, targetOffset, targetByteSize, wBuf);
        targetOffset += targetByteSize;
    }
    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "T=[{}] Completed", GetTimestampUs());
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
    free(wBuf);
}

TEST_F(UtMetaFsPerf, FullPageRead_Sync_QD1)
{
    issuedAioCnt = 0;
    completedAioCnt = 0;

    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    size_t fileSize = 1024 * 1024 * 100; // 100MB file

    int fd = CreateFileAndOpen(fileName, fileSize);

    uint32_t dataChunkSize = metaFsMgr.util.GetAlignedFileIOSize(fd);
    EXPECT_NE(dataChunkSize, 0);

    FileSizeType targetOffset = 0;
    FileSizeType targetByteSize = dataChunkSize * 1; // single page read

    uint8_t* rBuf = (uint8_t*)calloc(1, targetByteSize);

    int it = fileSize / targetByteSize;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;
    EnableAIR();

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "T=[{}] Start full page read. Iteration={}", GetTimestampUs(), it);
    while (it--)
    {
        rc_io = metaFsMgr.io.Read(fd, targetOffset, targetByteSize, rBuf);
        targetOffset += targetByteSize;
    }
    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "T=[{}] Completed", GetTimestampUs());
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
    free(rBuf);
}

TEST_F(UtMetaFsPerf, AIO_4KB_Rand_Read_AIO_QD1)
{
    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    size_t fileSize = 1024 * 1024 * 100; // 100MB file
    int fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    client.RunMultiQDAIOTest(fd, fileSize, MetaFsIoOpcode::Read, 1);
}

TEST_F(UtMetaFsPerf, AIO_4KB_Overwrite_QD4)
{
    uint32_t maxQD = 4;

    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    size_t fileSize = 1024 * 1024; // 1MB file
    FileFDType fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    uint8_t* wBuf = (uint8_t*)malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    std::generate_n(wBuf, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES, Sequential_data_pattern_gen);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;
    uint64_t soffset = 0;
    for (uint32_t qd = 0; qd < maxQD; qd++)
    {
        MetaFsAioCbCxt* aiocb = new MetaFsAioCbCxt(MetaFsIoOpcode::Write,
            fd,
            soffset,
            MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES, wBuf,
            AsEntryPointParam1(&TestClient::HandleIOCallback, &client));
        issuedAioCnt++;
        rc_io = metaFsMgr.io.SubmitIO(aiocb);

        if (rc_io.sc != IBOF_EVENT_ID::SUCCESS)
        {
            EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
            break;
        }
    }

    while (issuedAioCnt != completedAioCnt)
    {
    }

    void* rBuf = malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    rc_io = metaFsMgr.io.Read(fd, 0, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES, rBuf);
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    // verify data
    int status = memcmp(wBuf, rBuf, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    if (0 != status)
    {
        std::cout << "data mismatch detected...Dump file..." << std::endl;
        DumpMetaBuffer("wdata.bin", wBuf, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
        DumpMetaBuffer("rdata.bin", rBuf, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    }
    EXPECT_EQ(status, 0);

    free(wBuf);
    free(rBuf);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
}

TEST_F(UtMetaFsPerf, AIO_4KB_Rand_Read_AIO_QD32)
{
    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    uint64_t fileSize = 1024 * 1024 * 100; // 100MB file
    FileFDType fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    client.RunMultiQDAIOTest(fd, fileSize, MetaFsIoOpcode::Read, 32);
}

TEST_F(UtMetaFsPerf, AIO_4KB_Rand_Read_AIO_QD128)
{
    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    uint64_t fileSize = 1024 * 1024 * 100; // 100MB file
    FileFDType fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    client.RunMultiQDAIOTest(fd, fileSize, MetaFsIoOpcode::Read, 128);
}

TEST_F(UtMetaFsPerf, AIO_Sub4KB_PartialWrite_QD128)
{
    uint32_t maxQD = 128 * 3;

    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    uint64_t fileSize = 1024 * 1024; // 1MB file
    FileFDType fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    uint32_t byteShiftSize = 10;

    uint8_t* wBuf = (uint8_t*)malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    std::generate_n(wBuf, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES, Sequential_data_pattern_gen);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;
    void* tempBuf[maxQD] = {nullptr};
    uint64_t soffset = 0;
    uint64_t totalWrittenSize = 0;
    for (uint32_t qd = 0; qd < maxQD; qd++)
    {
        tempBuf[qd] = malloc(byteShiftSize);
        memcpy(tempBuf[qd], wBuf + (byteShiftSize * qd), byteShiftSize);
        MetaFsAioCbCxt* aiocb = new MetaFsAioCbCxt(MetaFsIoOpcode::Write,
            fd,
            soffset,
            byteShiftSize, tempBuf[qd],
            AsEntryPointParam1(&TestClient::HandleIOCallback, &client));
        issuedAioCnt++;
        rc_io = metaFsMgr.io.SubmitIO(aiocb);
        // rc_io = metaFsMgr.io.Write(fd, soffset, byteShiftSize, tempBuf[qd]);

        if (rc_io.sc != IBOF_EVENT_ID::SUCCESS)
        {
            EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
            break;
        }
        soffset += byteShiftSize;
        totalWrittenSize += byteShiftSize;
    }

    while (issuedAioCnt != completedAioCnt)
    {
    }

    void* rBuf = malloc(totalWrittenSize);
    rc_io = metaFsMgr.io.Read(fd, 0, totalWrittenSize, rBuf);
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    // verify data
    int status = memcmp(wBuf, rBuf, totalWrittenSize);
    if (0 != status)
    {
        std::cout << "data mismatch detected...Dump file..." << std::endl;
        DumpMetaBuffer("wdata.bin", wBuf, totalWrittenSize);
        DumpMetaBuffer("rdata.bin", rBuf, totalWrittenSize);
    }
    EXPECT_EQ(status, 0);

    for (uint32_t qd = 0; qd < maxQD; qd++)
    {
        free(tempBuf[qd]);
    }

    free(wBuf);
    free(rBuf);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
}

void
TestClient::RunMultiQDAIOTest(FileFDType fd, FileSizeType ioRange, MetaFsIoOpcode opcode, uint32_t maxQD)
{
    FileSizeType fileSize = ioRange;

    uint32_t dataChunkSize = metaFsMgr.util.GetAlignedFileIOSize(fd);
    EXPECT_NE(dataChunkSize, 0);

    FileSizeType targetByteSize = dataChunkSize * 1; // single page read

    std::vector<uint8_t*> rBuf(maxQD);

    for (uint32_t idx = 0; idx < maxQD; idx++)
    {
        rBuf[idx] = (uint8_t*)calloc(1, targetByteSize);
    }

    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "T=[{}] Start full page read (async).", GetTimestampUs());
    uint32_t soffset;
    volatile uint32_t qIdx = 0;

    UtMetaFsPerf::EnableAIR();
    while (maxQD > qIdx)
    {
        soffset = ((std::rand() % (fileSize / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES)) - 1) * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
        MetaFsAioCbCxt* aiocb = new MetaFsAioCbCxt(opcode,
            fd,
            soffset,
            targetByteSize, rBuf[qIdx],
            AsEntryPointParam1(&TestClient::HandleIOCallback, this));
        issuedAioCnt++;
        rc_io = metaFsMgr.io.SubmitIO(aiocb);
        if (rc_io.sc != IBOF_EVENT_ID::SUCCESS)
        {
            std::cout << "errror???" << std::endl;
            EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
            break;
        }

        qIdx++;
        currQD++;
    }

    const int totalIter = maxQD * 3000;
    while (issuedAioCnt < totalIter)
    {
        if (currQD < maxQD)
        {
            MetaFsAioCbCxt* aiocb;
            {
                SPIN_LOCK_GUARD_IN_SCOPE(availableQIdxLock);
                aiocb = availableAioCb.back();
                assert(aiocb != nullptr);
                availableAioCb.pop_back();
            }

            rc_io = metaFsMgr.io.SubmitIO(aiocb);
            if (rc_io.sc != IBOF_EVENT_ID::SUCCESS)
            {
                EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
                break;
            }
            issuedAioCnt++;
            currQD++;
        }
        std::this_thread::yield();
    }
    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Stop to send AIO now...");
    stopFlag = true;
    while (issuedAioCnt != completedAioCnt)
    {
    }

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "T=[{}] Completed", GetTimestampUs());
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

    for (uint32_t idx = 0; idx < maxQD; idx++)
    {
        free(rBuf[idx]);
    }
}

void
TestClient::HandleIOCallback(void* data)
{
    MetaFsAioCbCxt* cxt = reinterpret_cast<MetaFsAioCbCxt*>(data);

    if (cxt->CheckIOError())
    {
        MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE, "io error");
    }

    // issue next io with same io info
    completedAioCnt++;

    if (stopFlag)
    {
        delete cxt;
    }
    else
    {
        SPIN_LOCK_GUARD_IN_SCOPE(availableQIdxLock);
        availableAioCb.push_back(cxt);

        currQD--;
    }
}
