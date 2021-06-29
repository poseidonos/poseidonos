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

#ifndef __UT_MIM_BASE_H__
#define __UT_MIM_BASE_H__

#include <fstream>
#include <string>

#include "gtest/gtest.h"
#include "mfs_ut_framework.h"

namespace pos
{
// class UtMIMBasic : public MetaFsUnitTestBase
class UtMIMBasic : public ::testing::Test
{
public:
    void
    SetUp(void) override
    {
        metaIoMgr.Init();
        metaIoMgr.Bringup();
        mvmTopMgr.SetModuleReady();
    }

    void
    TearDown(void) override
    {
        metaIoMgr.Finalize();
        metaIoMgr.SetModuleHalt();
        mvmTopMgr.SetModuleHalt();
    }

    void
    CreateDummyFile(FileSizeType fileSize, std::string arrayName)
    {
        POS_EVENT_ID sc;
        MetaFsFileControlRequest req;
        req.reqType = MetaFsFileControlType::FileCreate;
        req.fileByteSize = fileSize;
        req.arrayName = &arrayName;
        sc = mvmTopMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
    }

    uint32_t
    OpenDummyFile(std::string arrayName)
    {
        uint32_t fd;

        POS_EVENT_ID sc;
        MetaFsFileControlRequest req;
        req.reqType = MetaFsFileControlType::FileOpen;
        std::string dummyFile("dummyFile.txt");
        req.fileName = &dummyFile;
        req.arrayName = &arrayName;
        sc = mvmTopMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);

        fd = req.completionData.openfd;

        return fd;
    }

    void
    CloseDummyFile(uint32_t fd, std::string arrayName)
    {
        POS_EVENT_ID sc;
        MetaFsFileControlRequest req;
        req.reqType = MetaFsFileControlType::FileClose;
        req.arrayName = &arrayName;
        sc = mvmTopMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
    }

    void
    DumpMetaBuffer(const char* fileName, void* buf, size_t byteSize)
    {
        std::string targetFile(std::string("/tmp/metaStorage/") + fileName);
        std::ofstream ofile(targetFile, std::ios::binary);
        ofile.write((char*)buf, byteSize);
        ofile.close();
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Dump finished..file={}", targetFile.c_str());
    }
};
} // namespace pos
#endif // __UT_MIM_BASE_H__
