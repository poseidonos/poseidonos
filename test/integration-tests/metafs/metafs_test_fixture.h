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

#include "src/metafs/metafs.h"
#include "src/metafs/storage/mss.h"

#include <unordered_map>
#include <string>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <memory>

#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"

using ::testing::NiceMock;

namespace pos
{
class FileIo
{
public:
    FileIo(void) = delete;
    explicit FileIo(const std::string& fileName)
    : fileName(fileName)
    {
        file.open(fileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    }
    virtual ~FileIo(void)
    {
        file.close();
    }
    virtual bool Write(void* const buf, const size_t byteOffset, const size_t byteSize)
    {
        file.seekp(byteOffset, file.beg);
        file.write(static_cast<char*>(buf), byteSize);
        if (!file.good())
        {
            return false;
        }
        return true;
    }
    virtual bool Read(void* buf, const size_t byteOffset, const size_t byteSize)
    {
        file.seekg(byteOffset);
        file.read((char*)buf, byteSize);
        if (!file.good())
        {
            return false;
        }
        return true;
    }

private:
    std::string fileName;
    std::fstream file;
};

class TestMetaStorageSubsystem : public MetaStorageSubsystem
{
public:
    explicit TestMetaStorageSubsystem(int arrayId) : MetaStorageSubsystem(arrayId)
    {
        fileNames.insert({MetaStorageType::SSD, "metafs_ssd.bin"});
        fileNames.insert({MetaStorageType::NVRAM, "metafs_nvm.bin"});

        for (auto& fileName : fileNames)
        {
            std::fstream create(fileName.second, std::ios::app);
            create.close();
            files.insert({fileName.first, std::make_shared<FileIo>(fileName.second)});
        }
    }
    virtual ~TestMetaStorageSubsystem(void)
    {
        for (auto& fileName : fileNames)
            std::remove(fileName.second.c_str());
    }
    POS_EVENT_ID CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag = false)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID Open(void)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID Close(void)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    uint64_t GetCapacity(MetaStorageType mediaType)
    {
        return 0;
    }
    POS_EVENT_ID ReadPage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID WritePage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    bool IsAIOSupport(void)
    {
        return true;
    }
    POS_EVENT_ID ReadPageAsync(MssAioCbCxt* cb)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID WritePageAsync(MssAioCbCxt* cb)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages)
    {
        return POS_EVENT_ID::SUCCESS;
    }
    LogicalBlkAddr TranslateAddress(MetaStorageType type, MetaLpnType theLpn)
    {
        return {};
    }
    POS_EVENT_ID DoPageIO(MssOpcode opcode, MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer,
        MetaLpnType numPages, uint32_t mpio_id, uint32_t tagid)
    {
        printf("DoPageIO, %s, mediaType: %d, startLpn: %ld, size: %ld, mpio_id: %d, tagid: %d, thread: %d\n",
            ((int)opcode == 0) ? "write" : "read ", (int)mediaType, metaLpn, numPages, mpio_id, tagid, sched_getcpu());

        assert(false);

        return POS_EVENT_ID::SUCCESS;
    }
    POS_EVENT_ID DoPageIOAsync(MssOpcode opcode, MssAioCbCxt* cb)
    {
        bool result = true;
        if (opcode == MssOpcode::Read)
            result = files[cb->GetIoContext()->media]->Read(
                cb->GetIoContext()->buf,
                cb->GetIoContext()->metaLpn * 4096,
                cb->GetIoContext()->lpnCnt * 4096);
        else
            result = files[cb->GetIoContext()->media]->Write(
                cb->GetIoContext()->buf,
                cb->GetIoContext()->metaLpn * 4096,
                cb->GetIoContext()->lpnCnt * 4096);

        printf("DoPageIOAsync, %s, result: %s, mediaType: %d, startLpn: %ld, size: %ld, mpio_id: %d, tagid: %d, thread: %d\n",
            ((int)opcode == 0) ? "write" : "read ", (result == true) ? "good" : "fail",
            (int)cb->GetIoContext()->media, cb->GetIoContext()->metaLpn, cb->GetIoContext()->lpnCnt,
            cb->GetIoContext()->mpioId, cb->GetIoContext()->tagId, sched_getcpu());

        if (!result)
            std::cout << "the request was failed" << std::endl;

        cb->InvokeCallback();

        return POS_EVENT_ID::SUCCESS;
    }

private:
    std::unordered_map<MetaStorageType, std::string> fileNames;
    std::unordered_map<MetaStorageType, std::shared_ptr<FileIo>> files;
};

class MetaFsTestFixture
{
public:
    MetaFsTestFixture(void);
    virtual ~MetaFsTestFixture(void);

protected:
    NiceMock<MockIArrayInfo>* arrayInfo = nullptr;

    MetaFs* metaFs = nullptr;

    MetaFsManagementApi* mgmt = nullptr;
    MetaFsFileControlApi* ctrl = nullptr;
    MetaFsIoApi* io = nullptr;
    MetaFsWBTApi* wbt = nullptr;
    TestMetaStorageSubsystem* storage = nullptr;
    NiceMock<MockTelemetryPublisher>* tpForMetaIo = nullptr;
    NiceMock<MockTelemetryPublisher>* tpForMetafs = nullptr;

    bool isLoaded = false;
    int arrayId = INT32_MAX;
    std::string arrayName = "";
    PartitionLogicalSize ptnSize[PartitionType::TYPE_COUNT];

private:
    void _SetArrayInfo(void);
    void _SetThreadModel(void);
    cpu_set_t _GetCpuSet(int from, int to);
};

} // namespace pos
