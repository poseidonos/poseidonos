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

#include "meta_file_intf_test.h"

#include "meta_file_include.h"

void
MetaFileTest::SetUp(void)
{
    if (metaFileStore->DoesFileExist() == true)
    {
        metaFileStore->Delete();
    }
    metaFileStore->Create(TEST_FILE_SIZE);
    metaFileStore->Open();
}

void
MetaFileTest::TearDown(void)
{
    metaFileStore->Close();
    delete metaFileStore;
}

void
MetaFileTest::_AsyncIoCallback(AsyncMetaFileIoCtx* ctx)
{
    asyncTestDoneCnt++;
    if (asyncTestCnt == asyncTestDoneCnt)
    {
        if (ctx->opcode == MetaFsIoOpcode::Read)
        {
            std::thread checkValidity(&MetaFileTest::_CheckReadValidity, this, ctx);
            checkValidity.detach();
        }
        else if (ctx->opcode == MetaFsIoOpcode::Write)
        {
            std::thread checkValidity(&MetaFileTest::_CheckWriteValidity, this);
            checkValidity.detach();
        }
    }

    delete ctx;
}

void
MetaFileTest::_CheckReadValidity(AsyncMetaFileIoCtx* ctx)
{
    // Check validity for previous write
    EXPECT_TRUE(memcmp(data, ctx->buffer, TEST_SIZE / 2) == 0);
    EXPECT_TRUE(memcmp(data2, ctx->buffer + TEST_SIZE / 2, TEST_SIZE / 2) == 0);

    checkValidityDone = true;
}

void
MetaFileTest::_CheckWriteValidity(void)
{
    // Check validity for previous write
    char* readData = new char[TEST_SIZE];
    metaFileStore->IssueIO(MetaFsIoOpcode::Read, 0, TEST_SIZE, readData);

    EXPECT_TRUE(memcmp(data, readData, TEST_SIZE / 2) == 0);
    EXPECT_TRUE(memcmp(data2, readData + TEST_SIZE / 2, TEST_SIZE / 2) == 0);
    delete[] readData;

    checkValidityDone = true;
}

void
MetaFileTest::_FileReadWrite(void)
{
    data = new char[TEST_SIZE / 2];
    memset(data, 0xAA, TEST_SIZE / 2);

    metaFileStore->IssueIO(MetaFsIoOpcode::Write, 0, TEST_SIZE / 2, data);

    char* readData = new char[TEST_SIZE];
    metaFileStore->IssueIO(MetaFsIoOpcode::Read, 0, TEST_SIZE, readData);

    EXPECT_TRUE(memcmp(data, readData, TEST_SIZE / 2) == 0);

    delete[] data;
    delete[] readData;
}

void
MetaFileTest::_FileReadWriteAppend(void)
{
    data = new char[TEST_SIZE];
    memset(data, 0xAA, TEST_SIZE / 2);
    memset(data + TEST_SIZE / 2, 0xBB, TEST_SIZE / 2);

    uint32_t offset = 0;
    metaFileStore->AppendIO(MetaFsIoOpcode::Write, offset, TEST_SIZE / 2, data);
    metaFileStore->AppendIO(MetaFsIoOpcode::Write, offset, TEST_SIZE / 2, data + TEST_SIZE / 2);

    char* readData = new char[TEST_SIZE];

    offset = 0;
    metaFileStore->AppendIO(MetaFsIoOpcode::Read, offset, TEST_SIZE / 2, readData);
    metaFileStore->AppendIO(MetaFsIoOpcode::Read, offset, TEST_SIZE / 2, readData + TEST_SIZE / 2);

    EXPECT_TRUE(memcmp(data, readData, TEST_SIZE) == 0);

    delete[] data;
    delete[] readData;
}

void
MetaFileTest::_AsyncFileRead(void)
{
    // Write data
    data = new char[TEST_SIZE / 2];
    memset(data, 0xCC, TEST_SIZE / 2);

    data2 = new char[TEST_SIZE / 2];
    memset(data2, 0xDD, TEST_SIZE / 2);

    metaFileStore->IssueIO(MetaFsIoOpcode::Write, 0, TEST_SIZE / 2, data);
    metaFileStore->IssueIO(MetaFsIoOpcode::Write, TEST_SIZE / 2, TEST_SIZE / 2, data2);

    // Test read
    asyncTestCnt = 1;
    asyncTestDoneCnt = 0;
    checkValidityDone = false;

    char* readData = new char[TEST_SIZE];
    AsyncMetaFileIoCtx* firstRead = new AsyncMetaFileIoCtx();
    firstRead->opcode = MetaFsIoOpcode::Read;
    firstRead->fd = metaFileStore->GetFd();
    firstRead->fileOffset = 0;
    firstRead->length = TEST_SIZE;
    firstRead->buffer = readData;
    firstRead->callback = std::bind(&MetaFileTest::_AsyncIoCallback, this, std::placeholders::_1);

    metaFileStore->AsyncIO(firstRead);

    while (checkValidityDone == false)
    {
    }

    delete[] data;
    delete[] data2;
    delete[] readData;
}

void
MetaFileTest::_AsyncFileWrite(void)
{
    // Test write
    data = new char[TEST_SIZE / 2];
    memset(data, 0xEE, TEST_SIZE / 2);

    data2 = new char[TEST_SIZE / 2];
    memset(data2, 0xFF, TEST_SIZE / 2);

    asyncTestCnt = 2;
    asyncTestDoneCnt = 0;
    checkValidityDone = false;

    AsyncMetaFileIoCtx* firstWrite = new AsyncMetaFileIoCtx();
    firstWrite->opcode = MetaFsIoOpcode::Write;
    firstWrite->fd = metaFileStore->GetFd();
    firstWrite->fileOffset = 0;
    firstWrite->length = TEST_SIZE / 2;
    firstWrite->buffer = data;
    firstWrite->callback = std::bind(&MetaFileTest::_AsyncIoCallback, this, std::placeholders::_1);

    metaFileStore->AsyncIO(firstWrite);

    AsyncMetaFileIoCtx* secondWrite = new AsyncMetaFileIoCtx();
    secondWrite->opcode = MetaFsIoOpcode::Write;
    secondWrite->fd = metaFileStore->GetFd();
    secondWrite->fileOffset = TEST_SIZE / 2;
    secondWrite->length = TEST_SIZE / 2;
    secondWrite->buffer = data2;
    secondWrite->callback = std::bind(&MetaFileTest::_AsyncIoCallback, this, std::placeholders::_1);

    metaFileStore->AsyncIO(secondWrite);

    while (checkValidityDone == false)
    {
    }

    delete[] data;
    delete[] data2;
}

void
MockFileTest::SetUp(void)
{
    metaFileStore = new MockFileIntf(filename);
    MetaFileTest::SetUp();
}

void
MockFileTest::TearDown(void)
{
    MetaFileTest::TearDown();
}

TEST_F(MockFileTest, FileReadWrite)
{
    _FileReadWrite();
}

TEST_F(MockFileTest, FileReadWriteAppend)
{
    _FileReadWriteAppend();
}

TEST_F(MockFileTest, AsyncFileRead)
{
    _AsyncFileRead();
}

TEST_F(MockFileTest, AsyncFileWrite)
{
    _AsyncFileWrite();
}

TEST_F(MockFileTest, GetFileSize)
{
    uint32_t size = metaFileStore->GetFileSize();
    EXPECT_TRUE(size == TEST_FILE_SIZE);
}
