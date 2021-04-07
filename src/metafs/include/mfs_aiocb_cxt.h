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

#include "os_header.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "mfs_ret_code.h"
#include "mfs_spinlock.h"
#include "mfs_type.h"
#include "src/io/general_io/volume_io.h"
#include "mfs_def.h"
#include "mfs_log.h"

using namespace ibofos;

class MetaFsAioCbCxt
{
public:
    using MetaFsAioCallbackPointer = std::function<void(void*)>;
    MetaFsAioCbCxt(MetaFsIoOpcode opcode, uint32_t fd, uint64_t soffset, uint64_t nbytes, void* buf, MetaFsAioCallbackPointer func)
    : opcode(opcode),
      fd(fd),
      soffset(soffset),
      nbytes(nbytes),
      buf(buf),
      callback(func),
      rc(IBOF_EVENT_ID::MFS_END),
      submitOk(false),
      tagId(0),
      callbackCount(0)
    {
    }
    MetaFsAioCbCxt(MetaFsIoOpcode opcode, uint32_t fd, void* buf, MetaFsAioCallbackPointer func)
    : opcode(opcode),
      fd(fd),
      soffset(0),
      nbytes(0),
      buf(buf),
      callback(func),
      rc(IBOF_EVENT_ID::MFS_END),
      submitOk(false),
      tagId(0),
      callbackCount(0)
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
        if (rc != IBOF_EVENT_ID::SUCCESS)
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
            rc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
        }
        else if (err.second == true)
        {
            rc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE;
        }
        else
        {
            rc = IBOF_EVENT_ID::SUCCESS;
        }
    }
    void
    InvokeCallback(void)
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Mio ][InvokeCb   ] type={}, req.tagId={}, status={}", opcode, tagId, rc);

        if (callback)
        {
            SPIN_LOCK_GUARD_IN_SCOPE(cbLock);
            callbackCount--;
            assert(callbackCount >= 0);

            if (0 == callbackCount)
                callback(this);
        }
    }

    MetaFsIoOpcode
    GetOpCode(void)
    {
        return opcode;
    }

    FileFDType
    GetFD(void)
    {
        return fd;
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
    SetUserIo(VolumeIoSmartPtr io)
    {
        userIo = io;
    }

    void
    SetCallbackCount(int cnt)
    {
        callbackCount = cnt;
    }

    int
    GetCallbackCount(void)
    {
        return callbackCount;
    }

private:
    friend class MetaFsIoApiWrapperClass;

    MetaFsIoOpcode opcode;
    FileFDType fd;
    FileSizeType soffset;
    FileSizeType nbytes;
    void* buf;
    MetaFsAioCallbackPointer callback;

    IBOF_EVENT_ID rc;
    bool submitOk;
    uint32_t tagId;
    VolumeIoSmartPtr userIo;
    int callbackCount;
    MetaFsSpinLock cbLock;
};
