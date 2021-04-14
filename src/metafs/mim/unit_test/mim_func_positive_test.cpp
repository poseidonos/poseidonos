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

#include "mim_func_positive_test.h"

#include <algorithm>
#include <string>
#include "meta_io_manager.h"
#include "metafs_config.h"

namespace pos
{
// 1. Check if MFS status is running  and storage mgmt is ready to execute I/O.
// MetaFsManagerBase <- MDI/MIM/MSC/MVM top mgr
TEST_F(UtMIMFunctionalPositive, CheckModuleReady)
{
    bool ret;
    ret = metaIoMgr.CheckModuleReadiness();
    EXPECT_EQ(ret, true);
}

// 2. Full IO (4032B) Sync. Read Operation at SSD
TEST_F(UtMIMFunctionalPositive, ProcessNewReq_FullRead)
{
    FileDescriptorType fd;
    FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 400KB
    std::string arrayName = "POSArray";

    // dummy file create & open
    CreateDummyFile(fileSize, arrayName);
    fd = OpenDummyFile(arrayName);

    MetaFsIoRequest req;

    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Read;
    req.targetMediaType = MetaStorageType::SSD;
    req.isFullFileIo = true;
    req.arrayName = arrayName;
    req.buf = malloc(fileSize); // Need to prepare sufficient buffer to read full file

    POS_EVENT_ID sc;
    sc = metaIoMgr.ProcessNewReq(req);
    free(req.buf);

    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
}

// 3. Full IO (4032B) Sync, Write operation at SSD
TEST_F(UtMIMFunctionalPositive, ProcessNewReq_FullWrite)
{
    FileDescriptorType fd = 0;
    FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 400KB
    std::string arrayName = "POSArray";

    CreateDummyFile(fileSize, arrayName);
    fd = OpenDummyFile(arrayName);

    MetaFsIoRequest req;

    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Write;
    req.targetMediaType = MetaStorageType::SSD;
    req.isFullFileIo = true;
    req.arrayName = arrayName;
    req.buf = malloc(fileSize);
    memset(req.buf, 0x85, fileSize);

    POS_EVENT_ID sc;
    sc = metaIoMgr.ProcessNewReq(req);
    free(req.buf);

    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
}

// 4. Aligned Partial IO Sync. Read
TEST_F(UtMIMFunctionalPositive, ProcessNewReq_AlignedPartialRead)
{
    FileDescriptorType fd = 0;
    FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 4KB * 100
    std::string arrayName = "POSArray";

    CreateDummyFile(fileSize, arrayName);
    fd = OpenDummyFile(arrayName);

    FileSizeType dataChunkSize;
    mvmTopMgr.GetDataChunkSize(fd, arrayName, dataChunkSize);

    // Offset  ~ (offset + size)
    FileSizeType fileReadOffset = dataChunkSize * 20;
    FileSizeType fileReadSize = dataChunkSize * 1;

    MetaFsIoRequest req;

    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Read;
    req.targetMediaType = MetaStorageType::SSD;
    req.byteOffsetInFile = fileReadOffset;
    req.byteSize = fileReadSize;
    req.isFullFileIo = false;
    req.arrayName = arrayName;
    req.buf = malloc(fileReadSize);

    POS_EVENT_ID sc;
    sc = metaIoMgr.ProcessNewReq(req); // MetaIoManager::_ProcessNewIoReq()

    free(req.buf);

    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
}

// 5. Unaligned Partial I/O Sync. Read
TEST_F(UtMIMFunctionalPositive, ProcessNewReq_UnalignedPartialRead)
{
    FileDescriptorType fd = 0;
    FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100; // 4KB * 100
    std::string arrayName = "POSArray";

    CreateDummyFile(fileSize, arrayName);
    fd = OpenDummyFile(arrayName);

    FileSizeType dataChunkSize;
    mvmTopMgr.GetDataChunkSize(fd, arrayName, dataChunkSize);

    FileSizeType fileReadOffset = 4;
    FileSizeType fileReadSize = 128;

    MetaFsIoRequest req;

    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Read;
    req.targetMediaType = MetaStorageType::SSD;
    req.byteOffsetInFile = fileReadOffset;
    req.byteSize = fileReadSize;
    req.isFullFileIo = false;
    req.arrayName = arrayName;
    req.buf = malloc(fileReadSize);

    POS_EVENT_ID sc;
    sc = metaIoMgr.ProcessNewReq(req); // MetaIoManager::_ProcessNewIoReq()

    free(req.buf);

    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
}

// 6. Aligned Patrial I/O Sync. Write
TEST_F(UtMIMFunctionalPositive, ProcessNewReq_AlignedPartialWrite)
{
    FileDescriptorType fd = 0;
    FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50; // 4KB * 50
    std::string arrayName = "POSArray";

    CreateDummyFile(fileSize, arrayName);
    fd = OpenDummyFile(arrayName);

    FileSizeType dataChunkSize;
    mvmTopMgr.GetDataChunkSize(fd, arrayName, dataChunkSize);

    FileSizeType fileWriteOffset = dataChunkSize * 5;
    FileSizeType fileWriteSize = dataChunkSize * 15;

    MetaFsIoRequest req;

    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Write;
    req.targetMediaType = MetaStorageType::SSD;
    req.byteOffsetInFile = fileWriteOffset;
    req.byteSize = fileWriteSize;
    req.isFullFileIo = false;
    req.arrayName = arrayName;
    req.buf = malloc(fileWriteSize);
    memset(req.buf, 0x85, fileWriteSize);

    POS_EVENT_ID sc;
    sc = metaIoMgr.ProcessNewReq(req); // MetaIoManager::_ProcessNewIoReq()
    free(req.buf);

    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
}

int
sequential_gen(void)
{
    static int i = 0;
    return ++i;
}

/*************************************************************
7. Write, read , and data compare
 a. sync write w/ file chunk size -> read -> data compare
 b. sync 512B read w/ aligned offset   -> data compare
 c. sync 512B read w/ unaligned offset -> data compare
************************************************************/
TEST_F(UtMIMFunctionalPositive, ProcessNewReq_PageWriteWithVerify)
{
    FileDescriptorType fd = 0;
    FileSizeType fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50; // 4KB * 50
    std::string arrayName = "POSArray";

    CreateDummyFile(fileSize, arrayName);
    fd = OpenDummyFile(arrayName);

    FileSizeType dataChunkSize;
    mvmTopMgr.GetDataChunkSize(fd, arrayName, dataChunkSize);

    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    std::generate_n(wBuf, dataChunkSize, sequential_gen);
    uint8_t* rBuf = (uint8_t*)calloc(1, fileSize);

    // 1 file chunk size
    FileSizeType fileWriteOffset = dataChunkSize * 5;
    FileSizeType fileWriteSize = dataChunkSize;

    MetaFsIoRequest req;

    // Write in a file chunk size
    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Write;
    req.targetMediaType = MetaStorageType::SSD;
    req.byteOffsetInFile = fileWriteOffset;
    req.byteSize = fileWriteSize;
    req.isFullFileIo = false;
    req.arrayName = arrayName;
    req.buf = wBuf;

    POS_EVENT_ID sc;
    sc = metaIoMgr.ProcessNewReq(req);
    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);

    // Read in a file chunk size
    req.fd = fd;
    req.ioMode = MetaIoMode::Sync;
    req.reqType = MetaIoRequestType::Read;
    req.targetMediaType = MetaStorageType::SSD;
    req.byteOffsetInFile = fileWriteOffset;
    req.byteSize = fileWriteSize; // same with read
    req.isFullFileIo = false;
    req.arrayName = arrayName;
    req.buf = rBuf;

    sc = metaIoMgr.ProcessNewReq(req);
    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);

    // data compare
    int ret = memcmp(wBuf, rBuf, fileWriteSize);
    EXPECT_EQ(ret, 0);

    if (ret != 0)
    { // data corruption
        DumpMetaBuffer("wBuf.bin", wBuf, fileWriteSize);
        DumpMetaBuffer("rBuf.bin", rBuf, fileWriteSize);
        free(wBuf);
        free(rBuf);
        return;
    }

    // Read in a 512B size with aligned offset
    {
        FileSizeType partialReadByteSize = 512;
        req.fd = fd;
        req.ioMode = MetaIoMode::Sync;
        req.reqType = MetaIoRequestType::Read;
        req.targetMediaType = MetaStorageType::SSD;
        req.byteOffsetInFile = fileWriteOffset;
        req.byteSize = partialReadByteSize; // same with read
        req.isFullFileIo = false;
        req.arrayName = arrayName;
        req.buf = rBuf;

        sc = metaIoMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
        int ret = memcmp(wBuf + partialReadByteSize, rBuf + partialReadByteSize, partialReadByteSize);
        EXPECT_EQ(ret, 0);

        if (ret != 0)
        { // data corruption
            DumpMetaBuffer("wBuf.bin", wBuf + partialReadByteSize, partialReadByteSize);
            DumpMetaBuffer("rBuf.bin", rBuf + partialReadByteSize, partialReadByteSize);
        }
    }

    // Read in a 512B size with unaligned offset (offset + byteShift)
    {
        FileSizeType partialReadByteSize = 512;
        FileSizeType byteShift = 50;
        req.fd = fd;
        req.ioMode = MetaIoMode::Sync;
        req.reqType = MetaIoRequestType::Read;
        req.targetMediaType = MetaStorageType::SSD;
        req.byteOffsetInFile = fileWriteOffset + byteShift;
        req.byteSize = partialReadByteSize; // same with read
        req.isFullFileIo = false;
        req.arrayName = arrayName;
        req.buf = rBuf;

        sc = metaIoMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);

        // data compare
        int ret = memcmp(wBuf + fileWriteOffset + byteShift,
            rBuf + fileWriteOffset + byteShift, partialReadByteSize);
        EXPECT_EQ(ret, 0);

        if (ret != 0)
        { // data corruption
            DumpMetaBuffer("wBuf.bin", wBuf + fileWriteOffset + byteShift, partialReadByteSize);
            DumpMetaBuffer("rBuf.bin", rBuf + fileWriteOffset + byteShift, partialReadByteSize);
        }
    }

    free(wBuf);
    free(rBuf);
}
} // namespace pos
