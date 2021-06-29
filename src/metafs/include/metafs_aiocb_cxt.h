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
#include <atomic>
#include "os_header.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "metafs_return_code.h"
#include "metafs_spinlock.h"
#include "metafs_type.h"
#include "src/bio/volume_io.h"
#include "metafs_def.h"
#include "metafs_log.h"

namespace pos
{
class MetaFsAioCbCxt
{
public:
    using MetaFsAioCallbackPointer = std::function<void(void*)>;
    MetaFsAioCbCxt(MetaFsIoOpcode opcode, uint32_t fd, std::string arrayName, uint64_t soffset, uint64_t nbytes, void* buf, MetaFsAioCallbackPointer func)
    : opcode(opcode),
      fd(fd),
      arrayName(arrayName),
      soffset(soffset),
      nbytes(nbytes),
      buf(buf),
      callback(func),
      rc(POS_EVENT_ID::MFS_END),
      submitOk(false),
      tagId(0)
    {
        callbackCount = 0;
    }

    MetaFsAioCbCxt(MetaFsIoOpcode opcode, uint32_t fd, std::string arrayName, void* buf, MetaFsAioCallbackPointer func)
    : opcode(opcode),
      fd(fd),
      arrayName(arrayName),
      soffset(0),
      nbytes(0),
      buf(buf),
      callback(func),
      rc(POS_EVENT_ID::MFS_END),
      submitOk(false),
      tagId(0)
    {
    }

    void
    SetTagId(uint32_t id)
    {
        this->tagId = id;
    }

    uint32_t
    GetTagId(void)
    {
        return tagId;
    }
    bool
    CheckIOError(void)
    {
        if (rc != POS_EVENT_ID::SUCCESS)
        {
            return true;
        }
        return false;
    }

    void
    SetErrorStatus(MfsError err)
    {
        // need more error handling
        if (err.first != 0)
        {
            rc = POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
        }
        else if (err.second == true)
        {
            rc = POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE;
        }
        else
        {
            rc = POS_EVENT_ID::SUCCESS;
        }
    }

    void
    InvokeCallback(void)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Mio ][InvokeCb   ] type={}, req.tagId={}, status={}", opcode, tagId, rc);

        if (callback)
        {
            int remainedCount = callbackCount.fetch_sub(1) - 1;
            assert(remainedCount >= 0);

            if (0 == remainedCount)
                callback(this);
        }
    }

    MetaFsIoOpcode
    GetOpCode(void)
    {
        return opcode;
    }

    FileDescriptorType
    GetFD(void)
    {
        return fd;
    }

    std::string
    GetarrayName(void)
    {
        return arrayName;
    }

    FileSizeType
    GetOffset(void)
    {
        return soffset;
    }

    FileSizeType
    GetByteSize(void)
    {
        return nbytes;
    }

    void*
    GetBuffer(void)
    {
        return buf;
    }

    void
    SetCallbackCount(int cnt)
    {
        callbackCount = cnt;
    }

private:
    friend class MetaFsIoApi;

    MetaFsIoOpcode opcode;
    FileDescriptorType fd;
    std::string arrayName;
    FileSizeType soffset;
    FileSizeType nbytes;
    void* buf;
    MetaFsAioCallbackPointer callback;

    POS_EVENT_ID rc;
    bool submitOk;
    uint32_t tagId;
    std::atomic<int> callbackCount;
};
} // namespace pos
