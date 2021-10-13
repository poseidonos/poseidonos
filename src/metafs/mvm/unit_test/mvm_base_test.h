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

#ifndef __UT_MVM_BASE_H__
#define __UT_MVM_BASE_H__

#include <chrono>
#include <cstdint>
#include <string>
#include <random>
#include "metafs_config.h"
#include "mfs_ut_framework.h"

namespace pos
{
class UtMVMBasic : public ::testing::TestWithParam<int> //, public ::testing::Test
{
public:
    void
    SetUp(void) override
    {
        arrayName = "POSArray";

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Start SetUp sequence...");
        mimTopMgr.SetModuleReady();

        uint64_t testVolumeByteSize = 1 * 1024 * 1024 * 1024;
        MetaLpnType testMaxMetaLpn = testVolumeByteSize / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;

        metaVolMgr.Init(MetaVolumeType::NvRamVolume, arrayName, testMaxMetaLpn);
        metaVolMgr.Init(MetaVolumeType::SsdVolume, arrayName, testMaxMetaLpn);
        metaVolMgr.Bringup();
        metaVolMgr.CreateVolume(MetaVolumeType::NvRamVolume, arrayName);
        metaVolMgr.CreateVolume(MetaVolumeType::SsdVolume, arrayName);
        metaVolMgr.Open(false, arrayName);
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "SetUp done...");
    }

    void
    TearDown(void) override
    {
        bool resetCxt = false;
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Start TearDown sequence...");
        metaVolMgr.Close(resetCxt, arrayName);
        mimTopMgr.SetModuleHalt();
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "TearDown done...");
    }

    POS_EVENT_ID
    CreateTestFile(std::string& fileName, FileSizeType fileSize, MetaVolumeType volumeType = MetaVolumeType::SsdVolume)
    {
        MetaFsFileControlRequest req;
        req.reqType = MetaFsFileControlType::FileCreate;
        req.fileName = &fileName;
        req.fileByteSize = fileSize;
        req.arrayName = &arrayName;

        if (volumeType == MetaVolumeType::NvRamVolume)
        {
            req.fileProperty.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
            req.fileProperty.ioOpType = MetaFileDominant::WriteDominant;
        }

        POS_EVENT_ID sc;
        sc = metaVolMgr.ProcessNewReq(req); // MetaVolumeManager::_HandleCreateFileReq()

        return sc;
    }

    POS_EVENT_ID
    OpenTestFile(std::string& fileName, FileDescriptorType& fd)
    {
        MetaFsFileControlRequest req;
        req.reqType = MetaFsFileControlType::FileOpen;
        req.fileName = &fileName;
        req.arrayName = &arrayName;

        POS_EVENT_ID sc;
        sc = metaVolMgr.ProcessNewReq(req); // MetaVolumeManager::_HandleOpenFileReq()
        fd = req.completionData.openfd;

        return sc;
    }

    POS_EVENT_ID
    CloseTestFile(FileDescriptorType& fd)
    {
        MetaFsFileControlRequest req;
        req.reqType = MetaFsFileControlType::FileClose;
        req.fd = fd;
        req.arrayName = &arrayName;

        POS_EVENT_ID sc;
        sc = metaVolMgr.ProcessNewReq(req);
        return sc;
    }

    int
    GetRandomNumber(int maxNumber)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, maxNumber);

        return dis(gen);
    }

private:
    std::string arrayName;
};
} // namespace pos

#endif // __UT_MVM_BASE_H__
