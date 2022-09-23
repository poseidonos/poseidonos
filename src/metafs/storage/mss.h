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

#include "meta_storage_specific.h"
#include "mss_opcode.h"
#include "mss_state.h"
#include "mss_aio_cb_cxt.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"

namespace pos
{
/* MetaStorageSubsystem Class
 * Base Class for MSS Layer
 * Provides API to Store Pages
 * Currently Only Two APIs 1. ReadPage, 2. WritePage are there. Will include more as mgmt will build.
 * */
class MetaStorageSubsystem
{
public:
    MetaStorageSubsystem(void) = delete;
    explicit MetaStorageSubsystem(const int arrayId);
    virtual ~MetaStorageSubsystem(void);

    virtual POS_EVENT_ID CreateMetaStore(const int arrayId, const MetaStorageType mediaType,
        const uint64_t capacity, const bool formatFlag = false) = 0;
    virtual POS_EVENT_ID Open(void) = 0;
    virtual POS_EVENT_ID Close(void) = 0;
    virtual uint64_t GetCapacity(const MetaStorageType mediaType) = 0;
    virtual POS_EVENT_ID ReadPage(const MetaStorageType mediaType, const MetaLpnType metaLpn,
        void* buffer, const MetaLpnType numPages) = 0;
    virtual POS_EVENT_ID WritePage(const MetaStorageType mediaType, const MetaLpnType metaLpn,
        void* buffer, const MetaLpnType numPages) = 0;
    virtual bool IsAIOSupport(void) = 0; // Asynchronos API's used with pstore
    virtual POS_EVENT_ID ReadPageAsync(MssAioCbCxt* ctx) = 0;
    virtual POS_EVENT_ID WritePageAsync(MssAioCbCxt* ctx) = 0;

    virtual POS_EVENT_ID TrimFileData(const MetaStorageType mediaType, const MetaLpnType startLpn,
        void* buffer, const MetaLpnType numPages) = 0;
    virtual LogicalBlkAddr TranslateAddress(const MetaStorageType type, const MetaLpnType theLpn) = 0;

    virtual POS_EVENT_ID DoPageIO(const MssOpcode opcode, const MetaStorageType mediaType,
        const MetaLpnType metaLpn, void* buffer, const MetaLpnType numPages,
        const uint32_t mpio_id, const uint32_t tagid);
    virtual POS_EVENT_ID DoPageIOAsync(const MssOpcode opcode, MssAioCbCxt* ctx);

protected:
    int arrayId;
};
} // namespace pos
