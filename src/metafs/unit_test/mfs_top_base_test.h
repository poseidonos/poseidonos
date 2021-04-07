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

#ifndef __UT_MFS_TOP_BASE_H__
#define __UT_MFS_TOP_BASE_H__

#include <string>

#include "gtest/gtest.h"
#include "mfs.h"
#include "mfs_fb_adapter.h"
#include "mfs_msc_top.h"
#include "mfs_ut_framework.h"

class UtMetaFsTop : public ::testing::Test // public MetaFsUnitTestBase
{
public:
    virtual void
    SetUp(void) override
    {
        MetaStorageMediaInfoList mediaInfoList;
        MetaStorageMediaInfo ssdInfo;
        ssdInfo.media = MetaStorageType::SSD;
        ssdInfo.mediaCapacity = (uint64_t)2 * 1024 * 1024 * 1024; // 5GB
        mediaInfoList.push_back(ssdInfo);
        MetaStorageMediaInfo nvramInfo;
        nvramInfo.media = MetaStorageType::NVRAM;
        nvramInfo.mediaCapacity = (uint64_t)512 * 1024 * 1024; // 1GB
        mediaInfoList.push_back(nvramInfo);

        metaFsMgr.Init(mediaInfoList);
    }
    virtual void
    TearDown(void) override
    {
        metaFsMgr.sys.Unmount();
    }

    void
    EstablishFilesystem(void)
    {
        MetaFsReturnCode<IBOF_EVENT_ID> rc_sys;

        rc_sys = metaFsMgr.sys.Create();
        EXPECT_EQ(rc_sys.sc, IBOF_EVENT_ID::SUCCESS);
        rc_sys = metaFsMgr.sys.Mount();
        EXPECT_EQ(rc_sys.sc, IBOF_EVENT_ID::SUCCESS);
    }

    const char*
    GetDummyFileName(void)
    {
        return "dummyFile.txt";
    }

    void
    CreateDummyFile(uint64_t fileSize)
    {
        IBOF_EVENT_ID sc;
        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileCreate;
        req.fileByteSize = fileSize;
        dummyFileName = GetDummyFileName();
        req.fileName = &dummyFileName;
        sc = mvmTopMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
    }

    int
    OpenDummyFile(void)
    {
        int fd;

        IBOF_EVENT_ID sc;
        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileOpen;
        std::string dummyFile(GetDummyFileName());
        req.fileName = &dummyFile;
        sc = mvmTopMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);

        fd = req.completionData.openfd;

        return fd;
    }

    void
    CloseDummyFile(uint32_t fd)
    {
        IBOF_EVENT_ID sc;
        MetaFsMoMReqMsg req;
        req.reqType = MetaFsMoMReqType::FileClose;
        req.fd = fd;
        sc = mvmTopMgr.ProcessNewReq(req);
        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
    }

private:
    std::string dummyFileName;
};

#endif // __UT_MFS_TOP_BASE_H__
