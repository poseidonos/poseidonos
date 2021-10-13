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

#include "mfs_functional_test.h"

#include <algorithm>
#include <fstream>
#include <string>
#include "metafs.h"
#include "metafs_common.h"
#include "metafs_common_const.h"
#include "metafs_config.h"
#include "metafs_log.h"
#include "metafs_time.h"

namespace pos
{
TEST_F(UtMetaFsTopFunctionalPositive, MetaFilesystemEstablish)
{
    EstablishFilesystem();
}

FileDescriptorType
UtMetaFsTopFunctionalPositive::CreateFileAndOpen(string& fileName, string& arrayName, FileSizeType fileSize)
{
    FileDescriptorType fd;
    const FileDescriptorType INVALID_FD = MetaFsCommonConst::INVALID_FD;

    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    // MetaFilePropertySet prop;
    // prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
    // prop.ioOpType = MetaFileDominant::WriteDominant;
    rc_mgmt = metaFs.ctrl.CreateVolume(fileName, arrayName, fileSize);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

    bool result = metaFs.mgmt.AddArray(arrayName);
    EXPECT_EQ(result, true);

    rc_mgmt = metaFs.ctrl.Open(fileName, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
    fd = rc_mgmt.returnData;
    EXPECT_NE(fd, INVALID_FD);

    size_t fileByteSize = metaFs.ctrl.GetFileSize(fd, arrayName);
    EXPECT_EQ(fileSize, fileByteSize);

    return fd;
}

void
UtMetaFsTopFunctionalPositive::UnmountFilesystem(string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;

    rc_sys = metaFs.mgmt.UnmountSystem(arrayName);
    EXPECT_EQ(rc_sys.sc, POS_EVENT_ID::SUCCESS);
}

int
Sequential_data_pattern_gen(void)
{
    static int i = 0;
    return ++i;
}

void
WriteDataAndClose(FileDescriptorType fd, string arrayName, FileSizeType startOffset, FileSizeType byteSize, void* wBuf)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    rc_io = metaFs.io.Write(fd, arrayName, startOffset, byteSize, wBuf);
    EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);

    rc_mgmt = metaFs.ctrl.Close(fd, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
}

void
DumpMetaBuffer(const char* fileName, const void* buf, size_t byteSize)
{
    string targetFile(string("/tmp/metaStorage/") + fileName);
    ofstream ofile(targetFile, ios::binary);
    ofile.write((char*)buf, byteSize);
    ofile.close();
    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Dump finished..file={}", targetFile.c_str());
}

bool
CheckDataPersistency(string fileName, string arrayName, FileSizeType ioOffset, FileSizeType ioByteSize, const void* wBuf)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;
    const FileDescriptorType INVALID_FD = MetaFsCommonConst::INVALID_FD;
    int fd;

    rc_mgmt = metaFs.ctrl.Open(fileName, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
    fd = rc_mgmt.returnData;
    EXPECT_NE(fd, INVALID_FD);

    uint8_t* rBuf = (uint8_t*)calloc(1, ioByteSize);
    memset(rBuf, 0, ioByteSize);

    rc_io = metaFs.io.Read(fd, arrayName, ioOffset, ioByteSize, rBuf);
    EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);

    rc_mgmt = metaFs.ctrl.Close(fd, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

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

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Verifying data...");
    if (ret != 0)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "Data miscompare detected!");
        DumpMetaBuffer("wBuf.bin", wBuf, byteSize);
        DumpMetaBuffer("rBuf.bin", rBuf, byteSize);

        return false;
    }
    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Done");

    return true;
}

TEST_F(UtMetaFsTopFunctionalPositive, TestLargeFile)
{
    EstablishFilesystem();

    const FileDescriptorType INVALID_FD = MetaFsCommonConst::INVALID_FD;
    string fileName = string("test/0");
    string arrayName = "POSArray";
    size_t fileByteSize = ((size_t)100 * 1024 * 1024); // 100MB
    MetaFilePropertySet newprop;

    uint32_t fd;
    {
        MetaFsReturnCode<POS_EVENT_ID> rc;
        rc = metaFs.ctrl.CreateVolume(fileName, arrayName, fileByteSize, newprop);
        EXPECT_EQ(rc.sc, POS_EVENT_ID::SUCCESS);
        bool result = metaFs.mgmt.AddArray(arrayName);
        EXPECT_EQ(result, true);
        rc = metaFs.ctrl.Open(fileName, arrayName);
        EXPECT_EQ(rc.sc, POS_EVENT_ID::SUCCESS);
        fd = rc.returnData;
        EXPECT_NE(fd, INVALID_FD);
    }
    {
        MetaFsReturnCode<POS_EVENT_ID> mgmtRC;
        uint32_t dataChunk = metaFs.ctrl.GetAlignedFileIOSize(fd, arrayName);
        uint32_t ioSize = dataChunk * 50;
        uint32_t byteOffset = dataChunk * 10;
        void* wBuf = calloc(1, fileByteSize);
        generate_n((uint8_t*)wBuf, fileByteSize, Sequential_data_pattern_gen);
        MetaFsReturnCode<POS_EVENT_ID> ioRC;

        ioRC = metaFs.io.Write(fd, arrayName, wBuf); // full file write
        EXPECT_EQ(ioRC.sc, POS_EVENT_ID::SUCCESS);
        mgmtRC = metaFs.ctrl.Close(fd, arrayName);
        EXPECT_EQ(mgmtRC.sc, POS_EVENT_ID::SUCCESS);

        // read some portion of file and verify
        CheckDataPersistency(fileName, arrayName, byteOffset, ioSize, (uint8_t*)wBuf + byteOffset);

        free(wBuf);
    }
}

TEST_F(UtMetaFsTopFunctionalPositive, MetaFileCreateAndRemountAsWellAsSomeIOs)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    EstablishFilesystem();

    string fileName = string("testfile" + to_string(GetTimestampUs()));
    string arrayName = "POSArray";
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50;
    int fd;
    fd = CreateFileAndOpen(fileName, arrayName, fileSize);

    uint32_t dataChunkSize = metaFs.ctrl.GetAlignedFileIOSize(fd, arrayName);
    EXPECT_NE(dataChunkSize, 0);
    FileSizeType targetOffset = dataChunkSize * 3;
    FileSizeType targetByteSize = dataChunkSize * 2;
    uint8_t* wBuf = (uint8_t*)calloc(1, targetByteSize);
    generate_n(wBuf, targetByteSize, Sequential_data_pattern_gen);

    WriteDataAndClose(fd, arrayName, targetOffset, targetByteSize, wBuf);
    CheckDataPersistency(fileName, arrayName, targetOffset, targetByteSize, wBuf);

    free(wBuf);
}

TEST_F(UtMetaFsTopFunctionalPositive, MetaFileCreateAndDoUnAlignedIO)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    EstablishFilesystem();

    string fileName = string("testfileX" + to_string(GetTimestampUs()));
    string arrayName = "POSArray";
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;
    int fd = CreateFileAndOpen(fileName, arrayName, fileSize);
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    // TEST1 - full multi-page io
    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Test - Full multi-page io");
    {
        uint32_t dataChunkSize = metaFs.ctrl.GetAlignedFileIOSize(fd, arrayName);
        EXPECT_NE(dataChunkSize, 0);

        uint32_t targetOffset = 140;
        uint32_t targetByteSize = dataChunkSize * 2;

        WriteDataAndClose(fd, arrayName, targetOffset, targetByteSize, wBuf + targetOffset);

        bool isSuccess = CheckDataPersistency(fileName, arrayName, targetOffset, targetByteSize, wBuf + targetOffset);

        if (!isSuccess)
        {
            goto FINALIZE;
        }
    }

    // TEST2 - partial page io
    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Test - Partial  page io");
    {
        int fd;
        const FileDescriptorType INVALID_FD = MetaFsCommonConst::INVALID_FD;
        rc_mgmt = metaFs.ctrl.Open(fileName, arrayName);
        EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
        fd = rc_mgmt.returnData;

        EXPECT_NE(fd, INVALID_FD);

        rc_io = metaFs.io.Write(fd, arrayName, 0, 4, wBuf);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
        rc_io = metaFs.io.Write(fd, arrayName, 4, 128, wBuf + 4);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
        rc_io = metaFs.io.Write(fd, arrayName, 132, 8, wBuf + 132);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
        rc_io = metaFs.io.Write(fd, arrayName, 140, 147456, wBuf + 140);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);

        uint8_t* rBuf = (uint8_t*)calloc(1, fileSize);
        rc_io = metaFs.io.Read(fd, arrayName, 0, 4, rBuf);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
        rc_io = metaFs.io.Read(fd, arrayName, 4, 128, rBuf + 4);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
        rc_io = metaFs.io.Read(fd, arrayName, 132, 8, rBuf + 132);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);
        rc_io = metaFs.io.Read(fd, arrayName, 140, 147456, rBuf + 140);
        EXPECT_EQ(rc_io.sc, POS_EVENT_ID::SUCCESS);

        bool isSuccess = VerifyData(wBuf, rBuf, 4 + 128 + 8 + 147456);
        free(rBuf);
        if (!isSuccess)
        {
            goto FINALIZE;
        }
    }

FINALIZE:
    free(wBuf);

    rc_mgmt = metaFs.ctrl.Close(fd, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
}

// MFS unmount case
// 1. MFS Unmount even though there are open files.
TEST_F(UtMetaFsTopFunctionalPositive, MFSUnmountCases)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    EstablishFilesystem();

    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;

    // 1st file create
    string fileName1 = string("testfileX" + to_string(GetTimestampUs()));
    string arrayName = "POSArray";
    int fd1 = CreateFileAndOpen(fileName1, arrayName, fileSize);

    // 2nd file create
    string fileName2 = string("testfileX" + to_string(GetTimestampUs()));
    int fd2 = CreateFileAndOpen(fileName2, arrayName, fileSize);

    // File Write
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    MetaFsReturnCode<POS_EVENT_ID> ioRC;
    ioRC = metaFs.io.Write(fd1, arrayName, wBuf); // full file write

    ioRC = metaFs.io.Write(fd2, arrayName, wBuf); // full file write

    // try MFS unmout enen though there are 2 active files
    ioRC = metaFs.mgmt.UnmountSystem(arrayName);
    EXPECT_NE(ioRC.sc, POS_EVENT_ID::SUCCESS);

    free(wBuf);

    // try MFS unmout enen though there are 1 active files
    rc_mgmt = metaFs.ctrl.Close(fd1, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

    ioRC = metaFs.mgmt.UnmountSystem(arrayName);
    EXPECT_NE(ioRC.sc, POS_EVENT_ID::SUCCESS);

    // try MFS unmout enen though there are 0 active files
    rc_mgmt = metaFs.ctrl.Close(fd2, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

    ioRC = metaFs.mgmt.UnmountSystem(arrayName);
    EXPECT_EQ(ioRC.sc, POS_EVENT_ID::SUCCESS);
}

// MFS unmount -> Trim meta space(virtually) -> mount (need to init/create)
TEST_F(UtMetaFsTopFunctionalPositive, MFSUnmountandCleanInit)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_sys;
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> rc_io;

    EstablishFilesystem();

    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;

    // 1st file create
    string fileName1 = string("testfileX" + to_string(GetTimestampUs()));
    string arrayName = "POSArray";
    int fd1 = CreateFileAndOpen(fileName1, arrayName, fileSize);

    // File Write
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    MetaFsReturnCode<POS_EVENT_ID> ioRC;
    ioRC = metaFs.io.Write(fd1, arrayName, wBuf); // full file write
    free(wBuf);

    // try MFS unmout enen though there are 1 active files
    rc_mgmt = metaFs.ctrl.Close(fd1, arrayName);
    EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

    ioRC = metaFs.mgmt.UnmountSystem(arrayName);
    EXPECT_EQ(ioRC.sc, POS_EVENT_ID::SUCCESS);

    // Since the meta space was virtually trimmed, the meta file mgmt should be initialized and created.
    bool isInitialized = false;

    UtMetaFsTop::SetUp();

    if (false == isInitialized)
    {
        rc_sys = metaFs.mgmt.CreateSystem(arrayName);
        EXPECT_EQ(rc_sys.sc, POS_EVENT_ID::SUCCESS);

        rc_sys = metaFs.mgmt.MountSystem(arrayName);
        EXPECT_EQ(rc_sys.sc, POS_EVENT_ID::SUCCESS);
    }

    UnmountFilesystem(arrayName);
}

// {MetaFS mgmt mount -> file create -> file close -> MetaFS mgmt unmount } x 50
TEST_F(UtMetaFsTopFunctionalPositive, MFSMountUnmountRepeat)
{
    MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;
    MetaFsReturnCode<POS_EVENT_ID> ioRC;

    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100;
    uint8_t* wBuf = (uint8_t*)calloc(1, fileSize);
    generate_n(wBuf, fileSize, Sequential_data_pattern_gen);

    int test_cnt = 50;

    for (int itr = 0; itr < test_cnt; itr++)
    {
        // init
        UtMetaFsTop::SetUp();

        // mfs create & mount
        EstablishFilesystem();

        // 1st file create
        string fileName1 = string("testfileX" + to_string(GetTimestampUs()));
        string arrayName = "POSArray";
        int fd1 = CreateFileAndOpen(fileName1, arrayName, fileSize);

        // File Write
        ioRC = metaFs.io.Write(fd1, arrayName, wBuf); // full file write

        // file close
        rc_mgmt = metaFs.ctrl.Close(fd1, arrayName);
        EXPECT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);

        // mfs unmount
        ioRC = metaFs.mgmt.UnmountSystem(arrayName);
        EXPECT_EQ(ioRC.sc, POS_EVENT_ID::SUCCESS);
    }

    free(wBuf);
}
} // namespace pos
