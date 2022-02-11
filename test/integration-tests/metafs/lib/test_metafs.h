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

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "test/integration-tests/metafs/lib/metafs_test_fixture.h"

using namespace std;

namespace pos
{
struct FileInformation
{
    std::string fileName;
    uint64_t fileSize;
    int fd;
    MetaFilePropertySet prop;
};

class BufferContext
{
public:
    BufferContext(const size_t size)
    : size_(size)
    {
        assert(size_ != 0);
        buffer_ = new char[size_];
        memset(buffer_, 0, size_);
    }
    ~BufferContext(void)
    {
        if (buffer_)
            delete buffer_;
    }
    void* GetBuffer(void) const
    {
        return (void*)buffer_;
    }

private:
    const size_t size_;
    char* buffer_;
};

class TestMetaFs : public MetaFsTestFixture, public ::testing::Test
{
public:
    TestMetaFs(void)
    : MetaFsTestFixture()
    {
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            files[arrayId].insert({StorageOpt::SSD, {"TestFileSsd", BYTE_4K * COUNT_OF_META_LPN_FOR_SSD, 0, {MetaFileAccessPattern::Default, MetaFileDominant::Default, MetaFileIntegrityType::Default}}});
            files[arrayId].insert({StorageOpt::NVRAM, {"TestFileNvm", BYTE_4K * COUNT_OF_META_LPN_FOR_NVM, 0, {MetaFileAccessPattern::ByteIntensive, MetaFileDominant::Default, MetaFileIntegrityType::Default}}});
        }

        writeBuf = new char[BYTE_4K];
        readBuf = new char[BYTE_4K];

        memset(writeBuf, 0, BYTE_4K);
        memset(readBuf, 0, BYTE_4K);
    }
    virtual ~TestMetaFs(void)
    {
        delete[] writeBuf;
        delete[] readBuf;
    }
    virtual void SetUp(void)
    {
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            // mount array
            ASSERT_EQ(0, GetMetaFs(arrayId)->Init());

            // create meta file
            MetaFsReturnCode<POS_EVENT_ID> rc_mgmt;

            for (auto& info : files[arrayId])
            {
                rc_mgmt = GetMetaFs(arrayId)->ctrl->Create(info.second.fileName,
                    info.second.fileSize, info.second.prop, info.first);
                ASSERT_EQ(rc_mgmt.sc, POS_EVENT_ID::SUCCESS);
                ASSERT_EQ(GetMetaFs(arrayId)->ctrl->Open(info.second.fileName, info.second.fd, info.first), POS_EVENT_ID::SUCCESS);
            }
        }

        doneCount = 0;
    }
    virtual void TearDown(void)
    {
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            for (auto& info : files[arrayId])
            {
                EXPECT_EQ(GetMetaFs(arrayId)->ctrl->Close(info.second.fd, info.first), POS_EVENT_ID::SUCCESS);
            }

            // unmount array
            GetMetaFs(arrayId)->Dispose();
        }
    }

    /* count callback */
    void DoneCallback(void* data)
    {
        doneCount--;
    }

    /* wait until all requests are done */
    bool WaitForDone(const size_t timeoutInMs) const
    {
        auto start = chrono::system_clock::now();
        while (doneCount)
        {
            usleep(100);
            auto end = chrono::system_clock::now();
            size_t elapsedTimeInMs = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            if (elapsedTimeInMs >= timeoutInMs)
                return false;
        }
        return true;
    }

    /* set granularity size */
    void SetGranularity(const size_t granularityByteSize)
    {
        this->granularityByteSize = granularityByteSize;
    }

    /* write data, async */
    bool WritePattern(const int arrayId, const size_t requestCount)
    {
        size_t countPerLpn = BYTE_4K / granularityByteSize;
        MetaLpnType lpnCount = (requestCount + countPerLpn) / countPerLpn;
        size_t remainedCount = requestCount;

        granularityIndex = 0;
        doneCount += requestCount;

        for (MetaLpnType lpn = 0; lpn < lpnCount; ++lpn)
        {
            for (size_t i = 0; i < std::min(remainedCount, countPerLpn); ++i)
            {
                FileSizeType startOffset = (lpn * BYTE_4K) + (i * granularityByteSize);
                POS_EVENT_ID result = GetMetaFs(arrayId)->io->SubmitIO(
                    _CreateRequests(arrayId, startOffset, _PopBuffer(arrayId, granularityIndex++)), MetaStorageType::NVRAM);

                if (result != POS_EVENT_ID::SUCCESS)
                {
                    EXPECT_EQ(result, POS_EVENT_ID::SUCCESS) << "write fail code: " << (int)result;
                    return false;
                }
            }
            remainedCount -= countPerLpn;
        }
    }

    /* write data, sync */
    bool WritePatternSync(const int arrayId, const size_t requestCount)
    {
        size_t countPerLpn = BYTE_4K / granularityByteSize;
        MetaLpnType lpnCount = (requestCount + countPerLpn) / countPerLpn;
        size_t remainedCount = requestCount;

        granularityIndex = 0;

        for (MetaLpnType lpn = 0; lpn < lpnCount; ++lpn)
        {
            if (!_WriteSingleMetaLpn(arrayId, lpn, std::min(remainedCount, countPerLpn)))
                return false;
            remainedCount -= countPerLpn;
        }
    }

    /* read data, sync */
    bool VerifyPattern(const int arrayId, const size_t requestCount)
    {
        size_t countPerLpn = BYTE_4K / granularityByteSize;
        MetaLpnType lpnCount = (requestCount + countPerLpn) / countPerLpn;
        size_t remainedCount = requestCount;

        granularityIndex = 0;

        for (MetaLpnType lpn = 0; lpn < lpnCount; ++lpn)
        {
            if (!_ReadAndVerify(arrayId, lpn, std::min(remainedCount, countPerLpn)))
                return false;
            remainedCount -= countPerLpn;
        }
        return true;
    }

    void CreateBuffers(const size_t requestCount)
    {
        bufferList.clear();
        for (size_t i = 0; i < requestCount; ++i)
        {
            bufferList.emplace_back(std::make_shared<BufferContext>(granularityByteSize));
        }
    }

    void DeleteBuffers(void)
    {
        bufferList.clear();
    }

protected:
    std::unordered_map<int, std::unordered_map<StorageOpt, FileInformation>> files;
    char* writeBuf;
    char* readBuf;

    size_t granularityByteSize;
    size_t granularityIndex;

    std::vector<MetaFsAioCbCxt*> requestList;
    std::vector<std::shared_ptr<BufferContext>> bufferList;

    const size_t BYTE_4K = 4032;
    const size_t COUNT_OF_META_LPN_FOR_SSD = 1000;
    const size_t COUNT_OF_META_LPN_FOR_NVM = 100;

    std::atomic<size_t> doneCount;

private:
    MetaFsAioCbCxt* _CreateRequests(const int arrayId, const FileSizeType startOffset, void* buffer)
    {
        MetaFsAioCbCxt* aiocb = new MetaFsAioCbCxt(
            MetaFsIoOpcode::Write, files[arrayId][StorageOpt::NVRAM].fd, arrayId,
            startOffset, granularityByteSize, buffer,
            AsEntryPointParam1(&TestMetaFs::DoneCallback, this));

        return aiocb;
    }

    void* _PopBuffer(const int arrayId, const size_t index)
    {
        std::shared_ptr<BufferContext> bufCxt = bufferList.at(index);
        *(size_t*)((char*)bufCxt->GetBuffer()) = arrayId;
        *(size_t*)((char*)bufCxt->GetBuffer() + sizeof(size_t)) = index;
        return bufCxt->GetBuffer();
    }

    bool _WriteSingleMetaLpn(const int arrayId, const MetaLpnType targetLpn, const size_t requestCount)
    {
        char* buf = writeBuf;
        for (size_t i = 0; i < requestCount; ++i)
        {
            *(size_t*)buf = arrayId;
            *(size_t*)(buf + sizeof(size_t)) = granularityIndex++;
            FileSizeType startOffset = targetLpn * BYTE_4K + (i * granularityByteSize);
            POS_EVENT_ID result = GetMetaFs(arrayId)->io->Write(files[arrayId][StorageOpt::NVRAM].fd, startOffset, granularityByteSize, buf, MetaStorageType::NVRAM);
            if (result != POS_EVENT_ID::SUCCESS)
            {
                EXPECT_EQ(result, POS_EVENT_ID::SUCCESS) << "write fail code: " << (int)result;
                return false;
            }
            buf += granularityByteSize;
        }
        return true;
    }

    bool _ReadAndVerify(const int arrayId, const MetaLpnType targetLpn, const size_t requestCount)
    {
        // make data
        char* buf = writeBuf;
        for (size_t i = 0; i < requestCount; ++i)
        {
            *(size_t*)buf = arrayId;
            *(size_t*)(buf + sizeof(size_t)) = granularityIndex++;
            buf += granularityByteSize;
        }

        // read
        POS_EVENT_ID result = GetMetaFs(arrayId)->io->Read(files[arrayId][StorageOpt::NVRAM].fd, targetLpn * BYTE_4K, BYTE_4K, readBuf, MetaStorageType::NVRAM);
        if (result != POS_EVENT_ID::SUCCESS)
        {
            EXPECT_EQ(result, POS_EVENT_ID::SUCCESS) << "read fail code: " << (int)result;
            return false;
        }

        // verify
        if (!std::equal(readBuf, readBuf + (requestCount * granularityByteSize), writeBuf))
        {
            EXPECT_TRUE(false) << "verify fail: " << *(size_t*)readBuf;
            return false;
        }

        return true;
    }
};
} // namespace pos
