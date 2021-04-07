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

#pragma once

#include <string>

#include "mfs_top_base_test.h"

class UtMetaFsWBT : public UtMetaFsTop
{
public:
    void
    SetUp(void) override
    {
        UtMetaFsTop::SetUp();
        EstablishFilesystem();
    }
    void
    TearDown(void) override
    {
        UtMetaFsTop::TearDown();
    }

    FileFDType
    CreateFileAndOpen(std::string& fileName, FileSizeType fileSize)
    {
        FileFDType fd;
        const FileFDType INVALID_FD = MetaFsCommonConst::INVALID_FD;

        MetaFsReturnCode<IBOF_EVENT_ID> rc_mgmt;
        rc_mgmt = metaFsMgr.mgmt.Create(fileName, fileSize);
        EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);

        rc_mgmt = metaFsMgr.mgmt.Open(fileName);
        EXPECT_EQ(rc_mgmt.sc, IBOF_EVENT_ID::SUCCESS);
        fd = rc_mgmt.returnData;
        EXPECT_NE(fd, INVALID_FD);

        FileSizeType fileByteSize = metaFsMgr.util.GetFileSize(fd);
        EXPECT_EQ(fileSize, fileByteSize);

        return fd;
    }

private:
};
