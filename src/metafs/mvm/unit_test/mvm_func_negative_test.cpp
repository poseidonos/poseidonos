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

#include "mvm_func_negative_test.h"
#include "metafs_common_const.h"

namespace pos
{
// assume, the vol.has been already closed when the vol has not opened.
TEST_F(UtMVMFunctionalNegative, CloseVolumeBeforeOpen)
{
    std::string arrayName = "POSArray";
    TearDown(); // There is setup step to open
    bool isVolOpened = metaVolMgr.GetVolOpenFlag(arrayName);
    EXPECT_EQ(isVolOpened, false);

    bool resetCxt = false;
    bool isSuccess = metaVolMgr.Close(resetCxt, arrayName);
    EXPECT_EQ(isSuccess, true);
}

// Duplicate file creation
TEST_F(UtMVMFunctionalNegative, DuplicateFileCreation)
{
    std::string fileName = std::string("testfile").append(std::to_string(GetRandomNumber(1024)));
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "fineName = {}", fileName.c_str());

    POS_EVENT_ID sc;
    sc = CreateTestFile(fileName, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 1000);
    EXPECT_EQ(sc, POS_EVENT_ID::SUCCESS);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Test file created sucessfully...");

    sc = CreateTestFile(fileName, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 30);
    EXPECT_EQ(sc, POS_EVENT_ID::MFS_FILE_CREATE_FAILED);
}

// vol open without file creation
TEST_F(UtMVMFunctionalNegative, AttemptingOpenForUncreatedFile)
{
    std::string fileName = std::string("testfile").append(std::to_string(GetRandomNumber(1024)));
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "fineName = {}", fileName.c_str());

    POS_EVENT_ID sc;
    FileDescriptorType fd;
    // attempting to open file without file creation
    sc = OpenTestFile(fileName, fd);
    EXPECT_NE(sc, POS_EVENT_ID::SUCCESS); // should be failed
}

// vol close with invalid file.
TEST_F(UtMVMFunctionalNegative, AttemptingCloseForInvalidFile)
{
    std::string fileName = std::string("testfile").append(std::to_string(GetRandomNumber(1024)));
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "fineName = ", fileName.c_str());

    POS_EVENT_ID sc;
    FileDescriptorType fd = 0x850024;
    // attempting to close file which is invalid file
    sc = CloseTestFile(fd);
    EXPECT_NE(sc, POS_EVENT_ID::SUCCESS); // should be failed
}
} // namespace pos
