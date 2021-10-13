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

#pragma once

#include <atomic>
#include <thread>

#include "gtest/gtest.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/meta_file_intf/mock_file_intf.h"

#ifdef MFS_TEST
#include "metafs.h"
#include "src/array/partition/partition.h"
#include "src/metafs/metafs_file_intf.h"
#endif

using namespace std;
using namespace pos;

const char* filename = "TestData";
const char* arrayname = "POSArray";

class MetaFileTest : public ::testing::Test
{
protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    void _FileReadWrite(void);
    void _FileReadWriteAppend(void);
    void _AsyncFileRead(void);
    void _AsyncFileWrite(void);

    void _CheckReadValidity(AsyncMetaFileIoCtx* data);
    void _CheckWriteValidity(void);
    void _AsyncIoCallback(AsyncMetaFileIoCtx* data);

    MetaFileIntf* metaFileStore;
    const uint64_t TEST_SIZE = 1024;
    const uint64_t TEST_FILE_SIZE = 65 * 1024;

    char* data;
    char* data2;

    int asyncTestCnt = 0;
    std::atomic<int> asyncTestDoneCnt;
    std::atomic<bool> checkValidityDone;
};

class MockFileTest : public MetaFileTest
{
protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};
