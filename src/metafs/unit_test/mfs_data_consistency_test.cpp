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

#include <string>
#include "mfs_data_consistency_test.h"
#include "metafs.h"
#include "mfs_functional_test.h"

namespace pos
{
static std::atomic<int> issuedAioCnt;
static std::atomic<int> completedAioCnt;

extern void DumpMetaBuffer(const char* fileName, const void* buf, size_t byteSize);

void
MetaFSNonFuncDataConsistency::HandleIOCallback(void* data)
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
MetaFSNonFuncDataConsistency::_Cleanup(void)
{
    issuedAioCnt = completedAioCnt = 0;
}

void
MetaFSNonFuncDataConsistency::FillupPattern(void* buf, size_t nbytes, int pattern)
{
    memset(buf, pattern, nbytes);
}

bool
MetaFSNonFuncDataConsistency::VerifyData(void* origin, void* exp, size_t nbytes)
{
    bool res = true;
    if (0 != memcmp(origin, exp, nbytes))
    {
        res = false;
    }

    return res;
}

// Test description #1:
// write N number of sub-4KB async and verify all afterwards
// The scenario generates multiple mpios over 1 LPN (AWIBOF-988 issue)
TEST_F(MetaFSNonFuncDataConsistency, VerifyDataMultiSub4KWrite)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    std::string arrayName = "POSArray";
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50; // 200KB file size
    int fd;
    CreateDummyFile(fileSize);
    fd = OpenDummyFile();

    bool result = metaFs.mgmt.AddArray(arrayName);
    EXPECT_EQ(result, true);

    uint32_t dataChunkSize = metaFs.ctrl.GetAlignedFileIOSize(fd, arrayName);
    EXPECT_NE(dataChunkSize, 0);

    static const int NUM_REQS = 4;
    static const int FIXED_WRITE_SIZE = 2048;

    assert(fileSize > NUM_REQS * FIXED_WRITE_SIZE);
    MetaFsAioCbCxt* aiocb[NUM_REQS];

    issuedAioCnt = 0;
    completedAioCnt = 0;

    uint8_t* wbuf = new uint8_t[NUM_REQS * FIXED_WRITE_SIZE];
    for (int reqId = 0; reqId < NUM_REQS; ++reqId)
    {
        uint8_t* curbuf = wbuf + (reqId * FIXED_WRITE_SIZE);
        MetaFSNonFuncDataConsistency::FillupPattern(curbuf, FIXED_WRITE_SIZE, reqId + 1);
        aiocb[reqId] = new MetaFsAioCbCxt(MetaFsIoOpcode::Write, fd, arrayName, reqId * FIXED_WRITE_SIZE, FIXED_WRITE_SIZE, curbuf,
            AsEntryPointParam1(&MetaFSNonFuncDataConsistency::HandleIOCallback, this));
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
    MetaFSNonFuncDataConsistency::FillupPattern(rbuf, FIXED_WRITE_SIZE, 0xA5A5A5A5);
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
    delete rbuf;
    delete wbuf;

    CloseDummyFile(fd);
}
} // namespace pos
