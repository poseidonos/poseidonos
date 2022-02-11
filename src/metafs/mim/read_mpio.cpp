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

#include "read_mpio.h"
#include "mfs_asynccb_cxt_template.h"
#include "os_header.h"

namespace pos
{
ReadMpio::ReadMpio(void* mdPageBuf)
: Mpio(mdPageBuf)
{
    assert(mdPageBuf != nullptr);
    _InitStateHandler();
}

void
ReadMpio::_InitStateHandler(void)
{
    RegisterStateHandler(MpAioState::Init,
        new MpioStateExecuteEntry(MpAioState::Init, AsMpioStateEntryPoint(&ReadMpio::_Init, this), MpAioState::Ready));
    RegisterStateHandler(MpAioState::Ready,
        new MpioStateExecuteEntry(MpAioState::Ready, AsMpioStateEntryPoint(&ReadMpio::_MakeReady, this), MpAioState::Read));
    RegisterStateHandler(MpAioState::Read,
        new MpioStateExecuteEntry(MpAioState::Read, AsMpioStateEntryPoint(&ReadMpio::DoIO, this), MpAioState::CheckReadStatus));
    RegisterStateHandler(MpAioState::CheckReadStatus,
        new MpioStateExecuteEntry(MpAioState::CheckReadStatus, AsMpioStateEntryPoint(&ReadMpio::CheckReadStatus, this), MpAioState::E2Echeck));
    RegisterStateHandler(MpAioState::E2Echeck,
        new MpioStateExecuteEntry(MpAioState::E2Echeck, AsMpioStateEntryPoint(&ReadMpio::DoE2ECheck, this), MpAioState::Complete));
    RegisterStateHandler(MpAioState::Error,
        new MpioStateExecuteEntry(MpAioState::Error, AsMpioStateEntryPoint(&ReadMpio::_HandleError, this), MpAioState::Error));
    RegisterStateHandler(MpAioState::Complete,
        new MpioStateExecuteEntry(MpAioState::Complete, AsMpioStateEntryPoint(&ReadMpio::_CompleteIO, this), MpAioState::Complete));
}

ReadMpio::~ReadMpio(void)
{
}

bool
ReadMpio::_Init(MpAioState expNextState)
{
    SetNextState(expNextState);
    return true;
}

bool
ReadMpio::_MakeReady(MpAioState expNextState)
{
    SetNextState(expNextState);
    return true;
}

bool
ReadMpio::_HandleError(MpAioState expNextState)
{
    MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_DATA_CORRUPTED,
        "[Mpio][RdMpioError] ReadMpio Error...req.tagId={}, mpio_id={}",
        io.tagId, io.mpioId);

    SetNextState(expNextState);
    return true;
}

bool
ReadMpio::_CompleteIO(MpAioState expNextState)
{
    bool contd2NextRun = _CopyToUserBuf();

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mpio][RdMpioDone ] ReadMpio Complete...req.tagId={}, mpio_id={}",
        io.tagId, io.mpioId);

    SetNextState(expNextState);

    mssIntf = nullptr;
    aioModeEnabled = false;

    return contd2NextRun;
}

bool
ReadMpio::_CopyToUserBuf(void)
{
    void* userBuf = GetUserDataBuf();
    void* mdpageBuf = GetMDPageDataBuf();
    uint32_t byteOffset = io.startByteOffset;
    uint32_t byteSize = io.byteSize;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mpio][CopyDat2Buf] Copy R data to user buf req.tagId={}, mpio_id={}, offsetInChunk={}, size={}",
        io.tagId, io.mpioId, byteOffset, byteSize);

    bool syncDone = _DoMemCpy(userBuf, (uint8_t*)mdpageBuf + byteOffset, byteSize);

    return syncDone;
}
} // namespace pos
