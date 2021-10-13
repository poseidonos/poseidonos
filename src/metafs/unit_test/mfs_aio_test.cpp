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

#include "mfs_aio_test.h"

#include <algorithm>
#include <string>
#include "metafs.h"

namespace pos
{
volatile std::atomic<int> issuedAioCnt;
volatile std::atomic<int> completedAioCnt;
void
UtMetaFsTopAIOPositive::HandleIOCallback(void* data)
{
    MetaFsAioCbCxt* cxt = reinterpret_cast<MetaFsAioCbCxt*>(data);
    if (cxt->CheckIOError())
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE, "io error");
    }

    delete cxt;
    completedAioCnt++;
    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE, "callback done");
}

void
UtMetaFsTopAIOPositive::FillupPattern(void* buf, size_t nbytes, int pattern)
{
    memset(buf, pattern, nbytes);
}

bool
UtMetaFsTopAIOPositive::VerifyData(void* origin, void* exp, size_t nbytes)
{
    bool res = true;
    if (0 != memcmp(origin, exp, nbytes))
    {
        res = false;
    }

    return res;
}

// Write 2 different files asyncronously
TEST_F(UtMetaFsTopAIOPositive, BasicAIOTest)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    // file mgmt create & mount
    EstablishFilesystem();

    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    std::string arrayName = "POSArray";
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50;
    int fd;
    // file mgmt create, open, and get file size
    fd = CreateFileAndOpen(fileName, arrayName, fileSize);

    // get data chunk size (ex. 4032B)
    uint32_t dataChunkSize = metaFs.ctrl.GetAlignedFileIOSize(fd, arrayName);
    EXPECT_NE(dataChunkSize, 0);

    // offset ~ (offset+Byte)
    // file 1 : 3 ~ 5
    // file 2 : 6 ~ 8
    FileSizeType targetOffset1 = dataChunkSize * 3;
    FileSizeType targetByteSize1 = dataChunkSize * 2;
    FileSizeType targetOffset2 = dataChunkSize * 6;
    FileSizeType targetByteSize2 = dataChunkSize * 2;
    uint8_t* wBuf1 = (uint8_t*)calloc(1, targetByteSize1);
    uint8_t* wBuf2 = (uint8_t*)calloc(1, targetByteSize2);
    {
        // pattern 0 1 2 3 ...
        std::generate_n(wBuf1, targetByteSize1, Sequential_data_pattern_gen);

        MetaFsAioCbCxt* aiocb = new MetaFsAioCbCxt(MetaFsIoOpcode::Write, fd, arrayName, targetOffset1, targetByteSize1, wBuf1,
            AsEntryPointParam1(&UtMetaFsTopAIOPositive::HandleIOCallback, this));
        issuedAioCnt++;
        rc_io = metaFs.io.SubmitIO(aiocb);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
    }

    {
        std::generate_n(wBuf2, targetByteSize2, Sequential_data_pattern_gen);

        MetaFsAioCbCxt* aiocb = new MetaFsAioCbCxt(MetaFsIoOpcode::Write, fd, arrayName, targetOffset2, targetByteSize2, wBuf2,
            AsEntryPointParam1(&UtMetaFsTopAIOPositive::HandleIOCallback, this));
        issuedAioCnt++;
        rc_io = metaFs.io.SubmitIO(aiocb);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
    }

    while (issuedAioCnt != completedAioCnt)
    {
    }

    rc_mgmt = metaFs.ctrl.Close(fd, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

    // open, read file, close, and verify data(memcmp)
    CheckDataPersistency(fileName, arrayName, targetOffset1, targetByteSize1, wBuf1);
    CheckDataPersistency(fileName, arrayName, targetOffset2, targetByteSize2, wBuf2);

    UnmountFilesystem(arrayName);
    free(wBuf1);
    free(wBuf2);
}

// To test partial I/O with async I/O operations.
TEST_F(UtMetaFsTopAIOPositive, PartialIOAsyncWrite)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    EstablishFilesystem();

    std::string fileName = std::string("testfileX" + std::to_string(GetTimestampUs()));
    std::string arrayName = "POSArray";
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 500;
    int fd = CreateFileAndOpen(fileName, arrayName, fileSize);
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    std::generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Test - Partial  page io");

    uint32_t dataChunkSize = metaFs.ctrl.GetAlignedFileIOSize(fd, arrayName);
    EXPECT_NE(dataChunkSize, 0);

    static const int NUM_REQS = 40000;
    static const int FIXED_WRITE_SIZE = 40; // 40B Write

    assert(fileSize > NUM_REQS * FIXED_WRITE_SIZE);
    MetaFsAioCbCxt* aiocb[NUM_REQS];

    issuedAioCnt = 0;
    completedAioCnt = 0;

    uint8_t* wbuf = new uint8_t[NUM_REQS * FIXED_WRITE_SIZE];
    for (int reqId = 0; reqId < NUM_REQS; ++reqId)
    {
        uint8_t* curbuf = wbuf + (reqId * FIXED_WRITE_SIZE);
        UtMetaFsTopAIOPositive::FillupPattern(curbuf, FIXED_WRITE_SIZE, reqId + 1);
        aiocb[reqId] = new MetaFsAioCbCxt(MetaFsIoOpcode::Write, fd, arrayName, reqId * FIXED_WRITE_SIZE, FIXED_WRITE_SIZE, curbuf,
            AsEntryPointParam1(&UtMetaFsTopAIOPositive::HandleIOCallback, this));
    }

    for (int reqId = 0; reqId < NUM_REQS; ++reqId)
    {
        metaFs.io.SubmitIO(aiocb[reqId]);
        issuedAioCnt++;
    }
    while (issuedAioCnt != completedAioCnt)
    {
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "AIO done. Do verify...");

    // data verify
    uint8_t* rbuf = new uint8_t[FIXED_WRITE_SIZE];
    UtMetaFsTopAIOPositive::FillupPattern(rbuf, FIXED_WRITE_SIZE, 0xA5A5A5A5);
    bool res;
    for (int reqId = 0; reqId < NUM_REQS; ++reqId)
    {
        metaFs.io.Read(fd, arrayName, reqId * FIXED_WRITE_SIZE, FIXED_WRITE_SIZE, rbuf);

        uint8_t* curbuf = wbuf + (reqId * FIXED_WRITE_SIZE);

        res = VerifyData(curbuf, rbuf, FIXED_WRITE_SIZE);
        EXPECT_EQ(res, true);

        if (!res)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
                "Data miscompare detected!!! reqId={}", reqId);
            DumpMetaBuffer("wBuf.bin", curbuf, FIXED_WRITE_SIZE);
            DumpMetaBuffer("rBuf.bin", rbuf, FIXED_WRITE_SIZE);
            break;
        }
    }
    if (res)
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "All data matched! No miscompare detected!!");
    }

    rc_mgmt = metaFs.ctrl.Close(fd, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

    delete rbuf;
    delete wbuf;

    UnmountFilesystem(arrayName);
}
} // namespace pos
