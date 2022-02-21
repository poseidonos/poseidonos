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

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>

#include "src/metafs/storage/mss.h"
#include "test/integration-tests/metafs/lib/metafs_file_io.h"

namespace pos
{
class TestMetaStorageSubsystem : public MetaStorageSubsystem
{
public:
    explicit TestMetaStorageSubsystem(int arrayId);
    virtual ~TestMetaStorageSubsystem(void);
    virtual POS_EVENT_ID CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag = false) override;
    virtual POS_EVENT_ID Open(void) override;
    virtual POS_EVENT_ID Close(void) override;
    virtual uint64_t GetCapacity(MetaStorageType mediaType) override;
    virtual POS_EVENT_ID ReadPage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages) override;
    virtual POS_EVENT_ID WritePage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages) override;
    virtual bool IsAIOSupport(void) override;
    virtual POS_EVENT_ID ReadPageAsync(MssAioCbCxt* cb) override;
    virtual POS_EVENT_ID WritePageAsync(MssAioCbCxt* cb) override;
    virtual POS_EVENT_ID TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages) override;
    virtual LogicalBlkAddr TranslateAddress(MetaStorageType type, MetaLpnType theLpn) override;
    virtual POS_EVENT_ID DoPageIO(MssOpcode opcode, MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer,
        MetaLpnType numPages, uint32_t mpio_id, uint32_t tagid) override;
    virtual POS_EVENT_ID DoPageIOAsync(MssOpcode opcode, MssAioCbCxt* cb) override;

private:
    std::unordered_map<MetaStorageType, std::string> fileNames;
    std::unordered_map<MetaStorageType, std::shared_ptr<MetaFsFileIo>> files;
    const size_t BYTE_4K = 4096;
};
} // namespace pos
