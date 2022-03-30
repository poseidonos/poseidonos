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

#include "src/metafs/common/metafs_common.h"
#include "src/metafs/include/meta_storage_specific.h"

namespace pos
{
class MssAioData
{
public:
    MssAioData(void)
    : arrayId(INT32_MAX),
      media(MetaStorageType::Default),
      metaLpn(MetaFsCommonConst::INVALID_META_LPN),
      lpnCnt(MetaFsCommonConst::INVALID_META_LPN),
      buf(nullptr),
      error(0),
      errorStopState(false),
      mpioId(0),
      tagId(0),
      offset(0)
    {
    }

    void Init(const int arrayId, const MetaStorageType media, const MetaLpnType metaLpn,
        const MetaLpnType lpnCnt, void* buf, const uint32_t mpioId, const uint32_t tagId,
        const FileSizeType offset)
    {
        this->arrayId = arrayId;
        this->media = media;
        this->metaLpn = metaLpn;
        this->lpnCnt = lpnCnt;
        this->buf = buf;
        this->error = 0;
        this->errorStopState = false;
        this->mpioId = mpioId;
        this->tagId = tagId;
        this->offset = offset;
    }

    virtual int GetArrayId(void) const
    {
        return arrayId;
    }
    virtual MetaStorageType GetStorageType(void) const
    {
        return media;
    }
    virtual MetaLpnType GetMetaLpn(void) const
    {
        return metaLpn;
    }
    virtual MetaLpnType GetLpnCount(void) const
    {
        return lpnCnt;
    }
    virtual void* GetBuffer(void)
    {
        return buf;
    }
    virtual int GetError(void) const
    {
        return error;
    }
    virtual void SetError(const int err)
    {
        error = err;
    }
    virtual bool GetErrorStopState(void) const
    {
        return errorStopState;
    }
    virtual void SetErrorStopState(const bool state)
    {
        errorStopState = state;
    }
    virtual uint32_t GetMpioId(void) const
    {
        return mpioId;
    }
    virtual uint32_t GetTagId(void) const
    {
        return tagId;
    }
    virtual FileSizeType GetOffset(void) const
    {
        return offset;
    }

private:
    int arrayId;
    MetaStorageType media;
    MetaLpnType metaLpn;
    MetaLpnType lpnCnt;
    void* buf;
    int error;
    bool errorStopState;
    uint32_t mpioId;
    uint32_t tagId;
    FileSizeType offset;
};
} // namespace pos
