/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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
#include <experimental/filesystem>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/metafs/include/meta_volume_type.h"
#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "src/meta_file_intf/rocksdb_metafs_intf.h"
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
    : SIZE(size)
    {
        assert(SIZE != 0);
        buffer_ = new char[SIZE];
        memset(buffer_, 0, SIZE);
    }
    ~BufferContext(void)
    {
        if (buffer_)
            delete[] buffer_;
    }
    void* GetBuffer(void) const
    {
        return (void*)buffer_;
    }

private:
    const size_t SIZE;
    char* buffer_;
};

class RocksDbTestMetaFs : public MetaFsTestFixture, public ::testing::Test
{
public:
    RocksDbTestMetaFs(void)
    : MetaFsTestFixture()
    {
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            files[arrayId].insert({MetaVolumeType::SsdVolume,
                {"TestFileSsd", BYTE_4K * COUNT_OF_META_LPN_FOR_SSD, 0,
                    {MetaFileIntegrityType::Default, MetaFileType::Journal}}});
        }

        writeBuf = new char[BYTE_4K];
        readBuf = new char[BYTE_4K];

        memset(writeBuf, 0, BYTE_4K);
        memset(readBuf, 0, BYTE_4K);
    }
    virtual ~RocksDbTestMetaFs(void)
    {
        delete[] writeBuf;
        delete[] readBuf;
    }
    virtual void SetUp(void)
    {
        std::experimental::filesystem::create_directory("/etc/pos/POSRaid");
        for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
        {
            for (auto& info : files[arrayId])
            {
                RocksDBMetaFsIntf* rocksdbMetaFs = new RocksDBMetaFsIntf(info.second.fileName, arrayId, nullptr, nullptr, MetaFileType::Map, MetaVolumeType::SsdVolume);
                std::string pathName = "/etc/pos/POSRaid/" + std::to_string(arrayId) + "_IntegrationRocksMeta";
                int createDirStatus = rocksdbMetaFs->CreateDirectory(pathName);
                ASSERT_EQ(createDirStatus, 0);
                int createRocksMeta = rocksdbMetaFs->SetRocksMeta(pathName);
                ASSERT_EQ(createRocksMeta, 0);
                int createRocksMetaFsIntf = rocksdbMetaFs->Create(BYTE_4K * COUNT_OF_META_LPN_FOR_SSD);
                ASSERT_EQ(createRocksMetaFsIntf, 0);
                rocksDBMetaFsList.push_back(rocksdbMetaFs);
            }
        }

        doneCount = 0;
    }
    virtual void TearDown(void)
    {
        for (int i = 0; i < rocksDBMetaFsList.size(); i++)
        {
            int deleteRocksMeta = rocksDBMetaFsList[i]->DeleteRocksMeta();
            ASSERT_EQ(deleteRocksMeta, 0);
            int deleteDirStatus = rocksDBMetaFsList[i]->DeleteDirectory();
            ASSERT_EQ(deleteDirStatus, 0);
        }
    }

    /* count callback */
    void DoneCallback(void* data)
    {
        doneCount--;
    }

protected:
    std::unordered_map<int, std::unordered_map<MetaVolumeType, FileInformation>> files;
    char* writeBuf;
    char* readBuf;

    size_t granularityByteSize;
    size_t granularityIndex;

    std::vector<MetaFsAioCbCxt*> requestList;
    std::vector<std::shared_ptr<BufferContext>> bufferList;

    const size_t BYTE_4K = 4032;
    const size_t COUNT_OF_META_LPN_FOR_SSD = 10000;
    const size_t COUNT_OF_META_LPN_FOR_NVM = 100;

    std::atomic<size_t> doneCount;

    std::vector<RocksDBMetaFsIntf*> rocksDBMetaFsList;

    AsyncMetaFileIoCtx* CreateRequest(MetaFsIoOpcode opcode, const int arrayId, const FileSizeType startOffset, const FileSizeType byteSize, void* buffer)
    {
        AsyncMetaFileIoCtx* req = new AsyncMetaFileIoCtx();
        req->opcode = opcode;
        req->fd = files[arrayId][MetaVolumeType::SsdVolume].fd;
        req->fileOffset = startOffset;
        req->length = byteSize;
        req->buffer = (char*)buffer;
        req->callback = std::bind(&RocksDbTestMetaFs::DoneCallback,
            this, std::placeholders::_1);
        return req;
    }
};
} // namespace pos
