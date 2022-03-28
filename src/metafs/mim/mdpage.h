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

#include "mdpage_control_info.h"
#include "os_header.h"

namespace pos
{
// MetaData Page
class MDPage
{
public:
    explicit MDPage(void* buf);
    virtual ~MDPage(void);

    virtual void AttachControlInfo(void);
    virtual void Make(const MetaLpnType metaLpn, const FileDescriptorType fd,
        const int arrayId, const uint64_t signature);
    virtual bool CheckValid(const int arrayId, const uint64_t signature) const;
    virtual bool CheckFileMismatch(const FileDescriptorType fd) const;
    virtual bool CheckLpnMismatch(const MetaLpnType srcLpn) const;
    virtual void ClearCtrlInfo(void);
    virtual uint8_t* GetDataBuf(void) const
    {
        return dataAll;
    }
    virtual size_t GetDefaultDataChunkSize(void) const
    {
        return MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    }
    virtual uint32_t GetMfsSignature(void) const
    {
        return ctrlInfo->mfsSignature;
    }

private:
    uint8_t* dataAll;
    MDPageControlInfo* ctrlInfo;
};
} // namespace pos
