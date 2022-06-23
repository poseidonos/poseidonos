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

#include <string>
#include "meta_storage_specific.h"
#include "metafs_common.h"
#include "os_header.h"
#include "src/metafs/mim/metafs_io_request.h"

namespace pos
{
class MpioIoInfo
{
public:
    MpioIoInfo(void)
    : tagId(0),
      mpioId(0),
      opcode(MetaIoOpcode::Max),
      fileType(MetaFileType::General),
      targetMediaType(MetaStorageType::Max),
      targetFD(MetaFsCommonConst::INVALID_FD),
      metaLpn(INVALID_LPN),
      startByteOffset(0),
      byteSize(0),
      pageCnt(1),
      arrayId(INT32_MAX),
      userBuf(nullptr),
      signature(UINT32_MAX)
    {
    }

    uint32_t tagId;
    uint32_t mpioId;
    MetaIoOpcode opcode;
    MetaFileType fileType;
    MetaStorageType targetMediaType;
    FileDescriptorType targetFD;
    MetaLpnType metaLpn;
    FileSizeType startByteOffset;
    FileSizeType byteSize;
    MetaLpnType pageCnt; // should be 1 since currently we only supports 4KB considering IO Parallelism for 4KB IO
    int arrayId;
    void* userBuf;
    uint64_t signature;

    static const MetaLpnType INVALID_LPN = UINT64_MAX;
    static const MetaLpnType MAX_MPIO_PAGE_CNT = 1;
};

class MpioTimeInfo
{
public:
    uint32_t timeTickIssued;
    uint32_t timeTickCompleted;

    static const uint32_t INVALID_TIME_TICK = 0;

private:
};
} // namespace pos
