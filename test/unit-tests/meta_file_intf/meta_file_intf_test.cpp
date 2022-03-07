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

#include "src/meta_file_intf/meta_file_intf.h"

#include <gtest/gtest.h>

namespace pos
{
class MetaFileIntfTester : public MetaFileIntf
{
public:
    MetaFileIntfTester(void)
    : MetaFileIntf()
    {
    }
    MetaFileIntfTester(std::string fname, int arrayId,
                        MetaVolumeType volumeType = MetaVolumeType::SsdVolume)
    : MetaFileIntf(fname, arrayId, volumeType)
    {
    }
    ~MetaFileIntfTester(void)
    {
    }
    int Create(uint64_t size)
    {
        return 0;
    }
    bool DoesFileExist(void)
    {
        return 0;
    }
    int Delete(void)
    {
        return 0;
    }
    uint64_t GetFileSize(void)
    {
        return 0;
    }
    int AsyncIO(AsyncMetaFileIoCtx* ctx)
    {
        return 0;
    }
    int CheckIoDoneStatus(void* data)
    {
        return 0;
    }
    int _Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return 1;
    }
    int _Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return 1;
    }

    void SetFd(int fd)
    {
        this->fd = fd;
    }
};

TEST(MetaFileIntfTester, CheckFileName)
{
    std::string fileName = "TESTFILE";
    int arrayId = 0;
    MetaFileIntfTester* file = new MetaFileIntfTester(fileName, 0);

    EXPECT_EQ(fileName, file->GetFileName());

    delete file;
}

TEST(MetaFileIntfTester, CheckFileDescriptor)
{
    std::string fileName = "TESTFILE";
    std::string arrayName = "TESTARRAY";
    MetaFileIntfTester* file = new MetaFileIntfTester();

    file->SetFd(100);

    EXPECT_EQ(100, file->GetFd());

    delete file;
}

TEST(MetaFileIntfTester, CheckIssueIO_Read)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Read;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->IssueIO(opType, fileOffset, length, buffer));

    delete file;
}

TEST(MetaFileIntfTester, CheckIssueIO_Write)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Write;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->IssueIO(opType, fileOffset, length, buffer));

    delete file;
}

TEST(MetaFileIntfTester, CheckAppendIO_Read)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Read;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->AppendIO(opType, fileOffset, length, buffer));

    delete file;
}

TEST(MetaFileIntfTester, CheckAppendIO_Write)
{
    MetaFileIntfTester* file = new MetaFileIntfTester();

    MetaFsIoOpcode opType = MetaFsIoOpcode::Write;
    uint64_t fileOffset = 20;
    uint64_t length = 10;
    char* buffer = nullptr;

    EXPECT_EQ(1, file->AppendIO(opType, fileOffset, length, buffer));

    delete file;
}
} // namespace pos
