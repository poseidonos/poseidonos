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

#include "mvm_func_positive_test.h"

#include "mdpage_control_info.h"
#include "mfs.h"
#include "mfs_common_const.h"
#include "mfs_io_config.h"
#include "mfs_time.h"

// meta vol. open
TEST_F(UtMVMFunctionalPositive, MetaVolumeOpen)
{
    bool resetCxt = false;
    metaVolMgr.Close(resetCxt);
    metaVolMgr.Open(false);
    EXPECT_EQ(metaVolMgr.IsModuleReady(), true);
}

// INSTANTIATE_TEST_CASE_P =>
// FileCreateMulParameterized/UtMVMFunctionalPositive.FileCreateMany/0
// FileCreateMulParameterized/UtMVMFunctionalPositive.FileCreateMany/1
// FileCreateMulParameterized/UtMVMFunctionalPositive.FileCreatePersistentCheckAfterVolumeClose/0
// FileCreateMulParameterized/UtMVMFunctionalPositive.FileCreatePersistentCheckAfterVolumeClose/1

// many file creation
INSTANTIATE_TEST_CASE_P(FileCreateMulParameterized,
    UtMVMFunctionalPositive,
    ::testing::Range(1, 10));
TEST_P(UtMVMFunctionalPositive, FileCreateMany)
{
    int repeat = GetParam();

    while (repeat--) // create random files by repeating
    {
        std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            " File name = ", fileName.c_str());

        IBOF_EVENT_ID sc;
        sc = CreateTestFile(fileName, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100);
        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Test file created sucessfully...");
    }
}

// vol open check after close
TEST_P(UtMVMFunctionalPositive, FileCreatePersistentCheckAfterVolumeClose)
{
    int repeat = GetParam();

    while (repeat--)
    {
        std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            " File name = ", fileName.c_str());

        IBOF_EVENT_ID sc;
        sc = CreateTestFile(fileName, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 100);
        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Test file created sucessfully...");
    }
    bool resetCxt = false;
    EXPECT_EQ(metaVolMgr.Close(resetCxt /*output*/), true);
    EXPECT_EQ(metaVolMgr.Open(false), true);
}

// file create -> open -> fd validity check
TEST_F(UtMVMFunctionalPositive, FileOpen)
{
    FileFDType fd;
    std::string fileName = std::string("testfile" + std::to_string(GetTimeStampInMs()));
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        " File name = ", fileName.c_str());

    IBOF_EVENT_ID sc;
    sc = CreateTestFile(fileName, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 400);
    EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file created sucessfully...");

    sc = OpenTestFile(fileName, fd);
    EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);

    const FileFDType invalidFD = MetaFsCommonConst::INVALID_FD;
    EXPECT_NE(fd, invalidFD);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file opened successfully...");

    sc = CloseTestFile(fd);
    EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file closed successfully...");
}

// file create -> open -> close
TEST_F(UtMVMFunctionalPositive, FileClose)
{
    IBOF_EVENT_ID ret;
    FileFDType fd;
    std::string fileName = std::string("testfile" + std::to_string(GetTimeStampInMs()));
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        " File name = ", fileName.c_str());

    ret = CreateTestFile(fileName, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 400);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file created sucessfully...");

    ret = OpenTestFile(fileName, fd);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file opened successfully...");

    ret = CloseTestFile(fd);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file closed successfully...");
}

// check the file is on NVRAM
void
_CheckIfFileOnNVRAM(FileFDType fd)
{
    IBOF_EVENT_ID ret;
    MetaStorageType outTargetMediaType;
    ret = mvmTopMgr.GetTargetMediaType(fd, outTargetMediaType);

    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);
    EXPECT_EQ(outTargetMediaType, MetaStorageType::NVRAM);
}

// check the file is on SSD
void
_CheckIfFileOnSSD(FileFDType fd)
{
    IBOF_EVENT_ID ret;
    MetaStorageType outTargetMediaType;
    ret = mvmTopMgr.GetTargetMediaType(fd, outTargetMediaType);

    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);
    EXPECT_EQ(outTargetMediaType, MetaStorageType::SSD);
}

void
_CheckAllocatedMedia(FileFDType fd, size_t NVRAMSize, size_t reqSize)
{
    if (reqSize < NVRAMSize)
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "The requested I/O is allocated in NVRAM: NVRAMSize=", NVRAMSize, " >  reqSize=", reqSize);

        _CheckIfFileOnNVRAM(fd);
    }
    else
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "The requested I/O is allocated in SSD: NVRAMSize=", NVRAMSize, " <= reqSize=", reqSize);

        _CheckIfFileOnSSD(fd);
    }
}

// File Create on NVRAM according to the property (ByteIntesive, SmallSizeBlockIo, WriteDominant, small size (<=64KB)
// The NVRAM size is about 1GB (2020/02/06).
TEST_F(UtMVMFunctionalPositive, TestFileCreateOnNVRAM)
{
    IBOF_EVENT_ID ret;
    FileFDType fd;

    std::string fileName = std::string("testfile" + std::to_string(GetTimeStampInMs()));
    MetaFsMoMReqMsg req;
    req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;

    size_t NVRAMSize = metaFsMgr.util.GetTheBiggestExtentSize(req.fileProperty);
    req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::Default;
    req.fileProperty.ioOpType = MDFilePropIoOpType::Default;
    size_t SSDSize = metaFsMgr.util.GetTheBiggestExtentSize(req.fileProperty);

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        " NVRAM size = ", NVRAMSize, " SSD Size = ", SSDSize);

    // 1. frequent-byte access file with write dominant workload characteristic
    {
        std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));

        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileCreate;
        req.fileName = &fileName;
        req.fileByteSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 400;
        req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;

        // file create on NVRAM
        ret = metaVolMgr.ProcessNewReq(req);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        // file open
        ret = OpenTestFile(fileName, fd);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        // Check media type is NVRAM.
        _CheckAllocatedMedia(fd, NVRAMSize, req.fileByteSize);
        NVRAMSize -= req.fileByteSize;

        CloseTestFile(fd);
    }

    // 2. small-size block io
    {
        std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileCreate;
        req.fileName = &fileName;
        req.fileByteSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 1024;
        req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::SmallSizeBlockIO;

        ret = metaVolMgr.ProcessNewReq(req);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        ret = OpenTestFile(fileName, fd);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        _CheckAllocatedMedia(fd, NVRAMSize, req.fileByteSize);
        NVRAMSize -= req.fileByteSize;
        CloseTestFile(fd);
    }

    // 3. write dominant io
    {
        std::string fileName = std::string("testfile" + std::to_string(GetTimestampUs()));
        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileCreate;
        req.fileName = &fileName;
        req.fileByteSize = (size_t)(SSDSize * 20) / 100; // 20%
        req.fileProperty.ioOpType = MDFilePropIoOpType::WriteDominant;

        ret = metaVolMgr.ProcessNewReq(req);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        ret = OpenTestFile(fileName, fd);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        _CheckAllocatedMedia(fd, NVRAMSize, req.fileByteSize);
        NVRAMSize -= req.fileByteSize;
        CloseTestFile(fd);
    }

    // 4. small file (<=64KB)
#if 0 // Allocating in NVRAM for the small write is deleted.
    {
        std::string fileName = std::string("testfile" + std::to_string(GetTimeStampInMs()));
        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileCreate;
        req.fileName = &fileName;
        req.fileByteSize = MetaFsConfig::TYPICAL_NVRAM_META_FILE_KB_SIZE * 1024;

        ret = metaVolMgr.ProcessNewReq(req);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        ret = OpenTestFile(fileName, fd);
        EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

        _CheckAllocatedMedia(fd, NVRAMSize, req.fileByteSize);
    }
#endif
}

// test for chunk size accoring to the data integtiry level
TEST_F(UtMVMFunctionalPositive, EstimateDataChunkSize)
{
    IBOF_EVENT_ID ret;
    MetaFsMoMReqMsg req;
    MetaFilePropertySet prop;
    prop.integrity = MDFilePropIntegrity::Lvl0_Disable;
    req.reqType = MetaFsMoMReqType::EstimateDataChunkSize;
    req.fileProperty = prop;
    ret = metaVolMgr.ProcessNewReq(req);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

    uint32_t exp_size = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    EXPECT_EQ(req.completionData.dataChunkSize, exp_size);
}

TEST_F(UtMVMFunctionalPositive, AlmostFullFileSizeTest)
{
    IBOF_EVENT_ID ret;
    MetaFsMoMReqMsg req;
    FileFDType fd1, fd2;

    req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;
    size_t NVRAMSize = metaFsMgr.util.GetTheBiggestExtentSize(req.fileProperty);

    req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::Default;
    req.fileProperty.ioOpType = MDFilePropIoOpType::Default;
    size_t SSDSize = metaFsMgr.util.GetTheBiggestExtentSize(req.fileProperty);

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        " NVRAM size = ", NVRAMSize, " SSD Size = ", SSDSize);

    req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;
    req.fileProperty.ioOpType = MDFilePropIoOpType::WriteDominant;
    req.fileProperty.integrity = MDFilePropIntegrity::Lvl0_Disable;
    uint32_t pageSize = metaFsMgr.util.EstimateAlignedFileIOSize(req.fileProperty);
    uint32_t maxSize = metaFsMgr.util.GetTheBiggestExtentSize(req.fileProperty);

    req.fileByteSize = (maxSize / pageSize - 1) * pageSize - (maxSize * 2) / 100; // almost 98% area.

    std::string fileName1 = std::string("testfile" + std::to_string(GetTimeStampInMs()));
    req.fileName = &fileName1;
    req.reqType = MetaFsMoMReqType::FileCreate;

    // file create on NVRAM
    ret = metaVolMgr.ProcessNewReq(req);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

    // file open
    ret = OpenTestFile(fileName1, fd1);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

    // create another file
    std::string fileName2 = std::string("testfile" + std::to_string(GetTimestampUs()));

    req.reqType = MetaFsMoMReqType::FileCreate;
    req.fileName = &fileName2;
    req.fileByteSize = (NVRAMSize * 5) / 100; // 5% of NVRAM size
    req.fileProperty.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;

    // file create on NVRAM
    ret = metaVolMgr.ProcessNewReq(req);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

    // file open
    ret = OpenTestFile(fileName2, fd2);
    EXPECT_EQ(ret, IBOF_EVENT_ID::SUCCESS);

    CloseTestFile(fd1);
    CloseTestFile(fd2);
}
