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

#include "mim_top_mock.h"

#include <unistd.h>

#include "mfs_log.h"
#include "mfs_mvm_top.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"

MockMetaIoMgr mockMetaIoMgr;
MetaFsMIMTopMgrClass& mimTopMgr = mockMetaIoMgr;

MockMetaIoMgr::MockMetaIoMgr(void)
{
}

MockMetaIoMgr::~MockMetaIoMgr(void)
{
}

MockMetaIoMgr&
MockMetaIoMgr::GetInstance(void)
{
    return mockMetaIoMgr;
}

void
MockMetaIoMgr::Init(void)
{
    // nothing required
    SetModuleInit();
}

bool
MockMetaIoMgr::Bringup(void)
{
    SetModuleReady();
    return true;
}

IBOF_EVENT_ID
MockMetaIoMgr::ProcessNewReq(MetaFsIoReqMsg& reqMsg)
{
    bool isSuccess = true;
    switch (reqMsg.reqType)
    {
        case MetaIoReqTypeEnum::Read:
        {
            FileSizeType ioByteSize, ioByteOffset;
            _SetFileIoInfo(reqMsg, ioByteSize, ioByteOffset);
            isSuccess = _ReadFile(reqMsg.fd, reqMsg.buf, ioByteSize, ioByteOffset);
        }
        break;

        case MetaIoReqTypeEnum::Write:
        {
            FileSizeType ioByteSize, ioByteOffset;
            _SetFileIoInfo(reqMsg, ioByteSize, ioByteOffset);
            isSuccess = _WriteFile(reqMsg.fd, reqMsg.buf, ioByteSize, ioByteOffset);
        }
        break;

        default:
        {
            return IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }

    if (!isSuccess)
    {
        return IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }
    else
    {
#if 0
        if (MetaIoModeEnum::Async == reqMsg.ioMode)
        {
            MetaFsAioCbCompleter* event =
                new MetaFsAioCbCompleter(
                        static_cast<MetaFsAioCbCxt*>(reqMsg.aiocb));
            ibofos::EventArgument::GetEventScheduler()->EnqueueEvent(event);
        }
#endif
    }

    return IBOF_EVENT_ID::SUCCESS;
};

void
MockMetaIoMgr::_SetFileIoInfo(const MetaFsIoReqMsg& reqMsg, FileSizeType& ioByteSize, FileSizeType& ioByteOffset)
{
    if (true == reqMsg.isFullFileIo)
    {
        mvmTopMgr.GetFileSize(reqMsg.fd, ioByteSize);
        ioByteOffset = 0;
    }
    else
    {
        ioByteSize = reqMsg.byteSize;
        ioByteOffset = reqMsg.byteOffsetInFile;
    }
}

bool
MockMetaIoMgr::_ReadFile(uint32_t fd, void* buf, FileSizeType byteSize, FileSizeType byteOffset)
{
    // what required to internal mock?
    return true;
}

bool
MockMetaIoMgr::_WriteFile(uint32_t fd, void* buf, FileSizeType byteSize, FileSizeType byteOffset)
{
    // what required to internal mock?
    return true;
}
