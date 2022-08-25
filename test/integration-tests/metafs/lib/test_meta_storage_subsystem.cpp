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

#include "test/integration-tests/metafs/lib/test_meta_storage_subsystem.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "src/metafs/storage/mss.h"
#include "test/integration-tests/metafs/lib/metafs_file_io.h"

namespace pos
{
TestMetaStorageSubsystem::TestMetaStorageSubsystem(int arrayId)
: MetaStorageSubsystem(arrayId)
{
    fileNames.insert({MetaStorageType::SSD, "metafs_ssd_" + std::to_string(arrayId) + ".bin"});
    fileNames.insert({MetaStorageType::NVRAM, "metafs_nvm_" + std::to_string(arrayId) + ".bin"});

    for (auto& fileName : fileNames)
    {
        std::fstream create(fileName.second, std::ios::app);
        create.close();
        files.insert({fileName.first, std::make_shared<MetaFsFileIo>(fileName.second)});
    }
}

TestMetaStorageSubsystem::~TestMetaStorageSubsystem(void)
{
    for (auto& fileName : fileNames)
        std::remove(fileName.second.c_str());
}

POS_EVENT_ID
TestMetaStorageSubsystem::CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
TestMetaStorageSubsystem::Open(void)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
TestMetaStorageSubsystem::Close(void)
{
    return EID(SUCCESS);
}

uint64_t
TestMetaStorageSubsystem::GetCapacity(MetaStorageType mediaType)
{
    return 0;
}

POS_EVENT_ID
TestMetaStorageSubsystem::ReadPage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
TestMetaStorageSubsystem::WritePage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
{
    return EID(SUCCESS);
}

bool
TestMetaStorageSubsystem::IsAIOSupport(void)
{
    return true;
}
POS_EVENT_ID
TestMetaStorageSubsystem::ReadPageAsync(MssAioCbCxt* cb)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
TestMetaStorageSubsystem::WritePageAsync(MssAioCbCxt* cb)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
TestMetaStorageSubsystem::TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages)
{
    return EID(SUCCESS);
}

LogicalBlkAddr
TestMetaStorageSubsystem::TranslateAddress(MetaStorageType type, MetaLpnType theLpn)
{
    return {};
}

POS_EVENT_ID
TestMetaStorageSubsystem::DoPageIO(MssOpcode opcode, MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer,
    MetaLpnType numPages, uint32_t mpio_id, uint32_t tagid)
{
    printf("DoPageIO, %s, mediaType: %d, startLpn: %ld, size: %ld, mpio_id: %d, tagid: %d, thread: %d\n",
        ((int)opcode == 0) ? "write" : "read ", (int)mediaType, metaLpn, numPages, mpio_id, tagid, sched_getcpu());

    throw logic_error("not implemented");

    return EID(SUCCESS);
}

POS_EVENT_ID
TestMetaStorageSubsystem::DoPageIOAsync(MssOpcode opcode, MssAioCbCxt* cb)
{
    bool result = true;
    if (opcode == MssOpcode::Read)
        result = files[cb->GetIoContext()->GetStorageType()]->Read(
            cb->GetIoContext()->GetBuffer(),
            cb->GetIoContext()->GetMetaLpn() * BYTE_4K,
            cb->GetIoContext()->GetLpnCount() * BYTE_4K);
    else
        result = files[cb->GetIoContext()->GetStorageType()]->Write(
            cb->GetIoContext()->GetBuffer(),
            cb->GetIoContext()->GetMetaLpn() * BYTE_4K,
            cb->GetIoContext()->GetLpnCount() * BYTE_4K);

    if (!result)
    {
        std::cout << "the request was failed" << std::endl;
        printf("DoPageIOAsync, %s, result: %s, mediaType: %d, startLpn: %ld, size: %ld, mpio_id: %d, tagid: %d, thread: %d\n",
            ((int)opcode == 0) ? "write" : "read ", (result == true) ? "good" : "fail",
            (int)cb->GetIoContext()->GetStorageType(), cb->GetIoContext()->GetMetaLpn(), cb->GetIoContext()->GetLpnCount(),
            cb->GetIoContext()->GetMpioId(), cb->GetIoContext()->GetTagId(), sched_getcpu());
    }

    cb->InvokeCallback();

    return EID(SUCCESS);
}
} // namespace pos
