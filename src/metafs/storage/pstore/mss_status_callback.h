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

#pragma once

#include <string>
#include "instance_tagid_allocator.h"
#include "meta_storage_specific.h"
#include "mfs_asynccb_cxt_template.h"
#include "metafs_common.h"
#include "metafs_type.h"
#include "os_header.h"
#include "src/logger/logger.h"

namespace pos
{
/**
 * Consumer of MssOnDisk will provide callback function
 * using "callbackPtr" function pointer only for aysnchronous operations
 *
 * Could have porvided simple function but needed to encapsulate it for testing.
 */

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
      tagId(0),
      mpioId(0),
      offset(0)
    {
    }

    void
    Init(int arrayId, MetaStorageType media, MetaLpnType metaLpn, MetaLpnType lpnCnt, void* buf, uint32_t id, uint32_t tagId, FileSizeType offset)
    {
        this->arrayId = arrayId;
        this->media = media;
        this->metaLpn = metaLpn;
        this->lpnCnt = lpnCnt;
        this->buf = buf;
        this->error = 0;
        this->errorStopState = false;
        this->mpioId = id;
        this->tagId = tagId;
        this->offset = offset;
    }

    int arrayId;
    MetaStorageType media;
    MetaLpnType metaLpn;
    MetaLpnType lpnCnt;
    void* buf;
    int error;
    bool errorStopState;
    uint32_t tagId;
    uint32_t mpioId;
    FileSizeType offset;
};

using MssCallbackPointer = AsyncCallback;

class MssAioCbCxt : public MetaAsyncCbCxt
{
public:
    MssAioCbCxt(void);

    void Init(MssAioData* cxt, MssCallbackPointer& callback);
    virtual ~MssAioCbCxt(void);
    void SaveIOStatus(int error);
    int GetArrayId(void)
    {
        return cxt->arrayId;
    }

private:
    MssAioData* cxt;
};
} // namespace pos
