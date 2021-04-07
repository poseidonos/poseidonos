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

#include "mfs_functional_test.h"

#include <algorithm>

#include "mfs.h"
#include "mfs_common.h"
#include "mfs_common_const.h"
#include "mfs_io_config.h"
#include "mfs_log.h"
#include "mfs_time.h"

TEST_F(UtMetaFsTopFunctionalPositive, MetaFilesystemEstablish)
{
    EstablishFilesystem();
}

FileFDType
UtMetaFsTopFunctionalPositive::CreateFileAndOpen(std::string& fileName, FileSizeType fileSize)
{
    FileFDType fd;
    const FileFDType INVALID_FD = MetaFsCommonConst::INVALID_FD;

    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    // MetaFilePropertySet prop;
    // prop.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;
    // prop.ioOpType = MDFilePropIoOpType::WriteDominant;
    rc_mgmt = metaFsMgr.mgmt.Create(fileName, fileSize);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

    rc_mgmt = metaFsMgr.mgmt.Open(fileName);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
    fd = rc_mgmt.returnData;
    EXPECT_NE(fd, INVALID_FD);

    size_t fileByteSize = metaFsMgr.util.GetFileSize(fd);
    EXPECT_EQ(fileSize, fileByteSize);

    return fd;
}

void
UtMetaFsTopFunctionalPositive::UnmountFilesystem(void)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_sys;

    rc_sys = metaFsMgr.sys.Unmount();
    EXPECT_EQ(rc_sys.sc, IBOF_EVENT_ID::SUCCESS);
}

int
Sequential_data_pattern_gen(void)
{
    static int i = 0;
    return ++i;
}

void
WriteDataAndClose(FileFDType fd, FileSizeType startOffset, FileSizeType byteSize, void* wBuf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    rc_io = metaFsMgr.io.Write(fd, startOffset, byteSize, wBuf);
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
}

#include <fstream>
void
DumpMetaBuffer(const char* fileName, const void* buf, size_t byteSize)
{
    std::string targetFile(std::string("/tmp/metaStorage/") + fileName);
    std::ofstream ofile(targetFile, std::ios::binary);
    ofile.write((char*)buf, byteSize);
    ofile.close();
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Dump finished..file={}", targetFile.c_str());
}

bool
CheckDataPersistency(std::string fileName, FileSizeType ioOffset, FileSizeType ioByteSize, const void* wBuf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;
    const FileFDType INVALID_FD = MetaFsCommonConst::INVALID_FD;
    int fd;

    rc_mgmt = metaFsMgr.mgmt.Open(fileName);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
    fd = rc_mgmt.returnData;
    EXPECT_NE(fd, INVALID_FD);

    uint8_t* rBuf = (uint8_t*)calloc(1, ioByteSize);
    memset(rBuf, 0, ioByteSize);

    rc_io = metaFsMgr.io.Read(fd, ioOffset, ioByteSize, rBuf);
    EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

    bool isSuccess = VerifyData(wBuf, rBuf, ioByteSize);
    EXPECT_EQ(isSuccess, true);

    free(rBuf);

    return isSuccess;
}

bool
VerifyData(const void* wBuf, const void* rBuf, FileSizeType byteSize)
{
    int ret = memcmp(wBuf, rBuf, byteSize);
    EXPECT_EQ(ret, 0);

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Verifying data...");
    if (ret != 0)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_ERROR_MESSAGE,
            "Data miscompare detected!");
        DumpMetaBuffer("wBuf.bin", wBuf, byteSize);
        DumpMetaBuffer("rBuf.bin", rBuf, byteSize);

        return false;
    }
    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Done");

    return true;
}

TEST_F(UtMetaFsTopFunctionalPositive, TestLargeFile)
{
    EstablishFilesystem();

    const FileFDType INVALID_FD = MetaFsCommonConst::INVALID_FD;
    std::string fileName = std::string("test/0");
    size_t fileByteSize = ((size_t)100 * 1024 * 1024); // 100MB
    MetaFilePropertySet newprop;

    uint32_t fd;
    {
        MetaFsReturnCode<IBOF_EVENT_ID> rc;
        rc = metaFsMgr.mgmt.Create(fileName, fileByteSize, newprop);
        EXPECT_EQ(rc.sc, IBOF_EVENT_ID::SUCCESS);
        rc = metaFsMgr.mgmt.Open(fileName);
        EXPECT_EQ(rc.sc, IBOF_EVENT_ID::SUCCESS);
        fd = rc.returnData;
        EXPECT_NE(fd, INVALID_FD);
    }
    {
        MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
        uint32_t dataChunk = metaFsMgr.util.GetAlignedFileIOSize(fd);
        uint32_t ioSize = dataChunk * 50;
        uint32_t byteOffset = dataChunk * 10;
        void* wBuf = calloc(1, fileByteSize);
        std::generate_n((uint8_t*)wBuf, fileByteSize, Sequential_data_pattern_gen);
        MetaFsReturnCode<IBOF_EVENT_ID> ioRC;

        ioRC = metaFsMgr.io.Write(fd, wBuf); // full file write
        EXPECT_EQ(ioRC.sc, IBOF_EVENT_ID::SUCCESS);
        mgmtRC = metaFsMgr.mgmt.Close(fd);
        EXPECT_EQ(mgmtRC.sc, IBOF_EVENT_ID::SUCCESS);

        // read some portion of file and verify
        CheckDataPersistency(fileName, byteOffset, ioSize, (uint8_t*)wBuf + byteOffset);

        free(wBuf);
    }
}

TEST_F(UtMetaFsTopFunctionalPositive, MetaFileCreateAndRemountAsWellAsSomeIOs)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_sys;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    EstablishFilesystem();

    std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50;
    int fd;
    fd = CreateFileAndOpen(fileName, fileSize);

    uint32_t dataChunkSize = metaFsMgr.util.GetAlignedFileIOSize(fd);
    EXPECT_NE(dataChunkSize, 0);
    FileSizeType targetOffset = dataChunkSize * 3;
    FileSizeType targetByteSize = dataChunkSize * 2;
    uint8_t* wBuf = (uint8_t*)calloc(1, targetByteSize);
    std::generate_n(wBuf, targetByteSize, Sequential_data_pattern_gen);

    WriteDataAndClose(fd, targetOffset, targetByteSize, wBuf);
    CheckDataPersistency(fileName, targetOffset, targetByteSize, wBuf);

    free(wBuf);
}

TEST_F(UtMetaFsTopFunctionalPositive, MetaFileCreateAndDoUnAlignedIO)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_sys;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    EstablishFilesystem();

    std::string fileName = std::string("testfileX" + std::to_string(GetTimestampUs()));
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;
    int fd = CreateFileAndOpen(fileName, fileSize);
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    std::generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    // TEST1 - full multi-page io
    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Test - Full multi-page io");
    {
        uint32_t dataChunkSize = metaFsMgr.util.GetAlignedFileIOSize(fd);
        EXPECT_NE(dataChunkSize, 0);

        uint32_t targetOffset = 140;
        uint32_t targetByteSize = dataChunkSize * 2;

        WriteDataAndClose(fd, targetOffset, targetByteSize, wBuf + targetOffset);

        bool isSuccess = CheckDataPersistency(fileName, targetOffset, targetByteSize, wBuf + targetOffset);

        if (!isSuccess)
        {
            goto FINALIZE;
        }
    }

    // TEST2 - partial page io
    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Test - Partial  page io");
    {
        int fd;
        const FileFDType INVALID_FD = MetaFsCommonConst::INVALID_FD;
        rc_mgmt = metaFsMgr.mgmt.Open(fileName);
        EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
        fd = rc_mgmt.returnData;

        EXPECT_NE(fd, INVALID_FD);

        rc_io = metaFsMgr.io.Write(fd, 0, 4, wBuf);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
        rc_io = metaFsMgr.io.Write(fd, 4, 128, wBuf + 4);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
        rc_io = metaFsMgr.io.Write(fd, 132, 8, wBuf + 132);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
        rc_io = metaFsMgr.io.Write(fd, 140, 147456, wBuf + 140);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

        uint8_t* rBuf = (uint8_t*)calloc(1, fileSize);
        rc_io = metaFsMgr.io.Read(fd, 0, 4, rBuf);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
        rc_io = metaFsMgr.io.Read(fd, 4, 128, rBuf + 4);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
        rc_io = metaFsMgr.io.Read(fd, 132, 8, rBuf + 132);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);
        rc_io = metaFsMgr.io.Read(fd, 140, 147456, rBuf + 140);
        EXPECT_EQ(rc_io.sc, IBOF_EVENT_ID::SUCCESS);

        bool isSuccess = VerifyData(wBuf, rBuf, 4 + 128 + 8 + 147456);
        free(rBuf);
        if (!isSuccess)
        {
            goto FINALIZE;
        }
    }

FINALIZE:
    free(wBuf);

    rc_mgmt = metaFsMgr.mgmt.Close(fd);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
}

// MFS unmount case
// 1. MFS Unmount even though there are open files.
TEST_F(UtMetaFsTopFunctionalPositive, MFSUnmountCases)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_sys;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    EstablishFilesystem();

    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;

    // 1st file create
    std::string fileName1 = std::string("testfileX" + std::to_string(GetTimestampUs()));
    int fd1 = CreateFileAndOpen(fileName1, fileSize);

    // 2nd file create
    std::string fileName2 = std::string("testfileX" + std::to_string(GetTimestampUs()));
    int fd2 = CreateFileAndOpen(fileName2, fileSize);

    // File Write
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    std::generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;
    ioRC = metaFsMgr.io.Write(fd1, wBuf); // full file write

    ioRC = metaFsMgr.io.Write(fd2, wBuf); // full file write

    // try MFS unmout enen though there are 2 active files
    ioRC = metaFsMgr.sys.Unmount();
    EXPECT_NE(ioRC.sc, IBOF_EVENT_ID::SUCCESS);

    free(wBuf);

    // try MFS unmout enen though there are 1 active files
    rc_mgmt = metaFsMgr.mgmt.Close(fd1);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

    ioRC = metaFsMgr.sys.Unmount();
    EXPECT_NE(ioRC.sc, IBOF_EVENT_ID::SUCCESS);

    // try MFS unmout enen though there are 0 active files
    rc_mgmt = metaFsMgr.mgmt.Close(fd2);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

    ioRC = metaFsMgr.sys.Unmount();
    EXPECT_EQ(ioRC.sc, IBOF_EVENT_ID::SUCCESS);
}

// MFS unmount -> Trim meta space(virtually) -> mount (need to init/create)
TEST_F(UtMetaFsTopFunctionalPositive, MFSUnmountandCleanInit)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_sys;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;

    EstablishFilesystem();

    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;

    // 1st file create
    std::string fileName1 = std::string("testfileX" + std::to_string(GetTimestampUs()));
    int fd1 = CreateFileAndOpen(fileName1, fileSize);

    // File Write
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    std::generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;
    ioRC = metaFsMgr.io.Write(fd1, wBuf); // full file write
    free(wBuf);

    // try MFS unmout enen though there are 1 active files
    rc_mgmt = metaFsMgr.mgmt.Close(fd1);
    EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

    ioRC = metaFsMgr.sys.Unmount();
    EXPECT_EQ(ioRC.sc, IBOF_EVENT_ID::SUCCESS);

    // Since the meta space was virtually trimmed, the meta file system should be initialized and created.
    bool isInitialized = false;

    UtMetaFsTop::SetUp();

    if (false == isInitialized)
    {
        rc_sys = metaFsMgr.sys.Create();
        EXPECT_EQ(rc_sys.sc, IBOF_EVENT_ID::SUCCESS);

        rc_sys = metaFsMgr.sys.Mount();
        EXPECT_EQ(rc_sys.sc, IBOF_EVENT_ID::SUCCESS);
    }

    UnmountFilesystem();
}

// {MetaFS system mount -> file create -> file close -> MetaFS system unmount } x 50
TEST_F(UtMetaFsTopFunctionalPositive, MFSMountUnmountRepeat)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;

    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    std::generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    int test_cnt = 50;

    for (int itr = 0; itr < test_cnt; itr++)
    {
        // init
        UtMetaFsTop::SetUp();

        // mfs create & mount
        EstablishFilesystem();

        // 1st file create
        std::string fileName1 = std::string("testfileX" + std::to_string(GetTimestampUs()));
        int fd1 = CreateFileAndOpen(fileName1, fileSize);

        // File Write
        ioRC = metaFsMgr.io.Write(fd1, wBuf); // full file write

        // file close
        rc_mgmt = metaFsMgr.mgmt.Close(fd1);
        EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

        // mfs unmount
        ioRC = metaFsMgr.sys.Unmount();
        EXPECT_EQ(ioRC.sc, IBOF_EVENT_ID::SUCCESS);
    }

    free(wBuf);
}
