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

#include "write_mpio.h"

namespace pos
{
WriteMpio::WriteMpio(void* mdPageBuf, const bool directAccessEnabled)
: Mpio(mdPageBuf, directAccessEnabled),
  prevLpn(0),
  currLpn(0),
  prevBuf(nullptr),
  currBuf(nullptr)
{
    if (!mdPageBuf)
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "The buffer is null.");
        assert(false);
    }
    _InitStateHandler();
}

void
WriteMpio::_InitStateHandler(void)
{
    RegisterStateHandler(MpAioState::Init,
        new MpioStateExecuteEntry(MpAioState::Init, AsMpioStateEntryPoint(&WriteMpio::_Init, this), MpAioState::Ready));
    RegisterStateHandler(MpAioState::Ready,
        new MpioStateExecuteEntry(MpAioState::Ready, AsMpioStateEntryPoint(&WriteMpio::_MakeReady, this), MpAioState::MergeData));
    RegisterStateHandler(MpAioState::Read,
        new MpioStateExecuteEntry(MpAioState::Read, AsMpioStateEntryPoint(&WriteMpio::DoIO, this), MpAioState::CheckReadStatus));
    RegisterStateHandler(MpAioState::CheckReadStatus,
        new MpioStateExecuteEntry(MpAioState::CheckReadStatus, AsMpioStateEntryPoint(&WriteMpio::CheckReadStatus, this), MpAioState::E2Echeck));
    RegisterStateHandler(MpAioState::E2Echeck,
        new MpioStateExecuteEntry(MpAioState::E2Echeck, AsMpioStateEntryPoint(&WriteMpio::DoE2ECheck, this), MpAioState::MergeData));
    RegisterStateHandler(MpAioState::MergeData,
        new MpioStateExecuteEntry(MpAioState::MergeData, AsMpioStateEntryPoint(&WriteMpio::_MergeData, this), MpAioState::PrepareWrite));
    RegisterStateHandler(MpAioState::PrepareWrite,
        new MpioStateExecuteEntry(MpAioState::PrepareWrite, AsMpioStateEntryPoint(&WriteMpio::_PrepareWrite, this), MpAioState::Write));
    RegisterStateHandler(MpAioState::Error,
        new MpioStateExecuteEntry(MpAioState::Error, AsMpioStateEntryPoint(&WriteMpio::_HandleError, this), MpAioState::Complete));
    RegisterStateHandler(MpAioState::Write,
        new MpioStateExecuteEntry(MpAioState::Write, AsMpioStateEntryPoint(&WriteMpio::_Write, this), MpAioState::CheckWriteStatus));
    RegisterStateHandler(MpAioState::CheckWriteStatus,
        new MpioStateExecuteEntry(MpAioState::CheckWriteStatus, AsMpioStateEntryPoint(&WriteMpio::CheckWriteStatus, this), MpAioState::Complete));
    RegisterStateHandler(MpAioState::Complete,
        new MpioStateExecuteEntry(MpAioState::Complete, AsMpioStateEntryPoint(&WriteMpio::_CompleteIO, this), MpAioState::Complete));
}

WriteMpio::~WriteMpio(void)
{
}

bool
WriteMpio::_Init(MpAioState expNextState)
{
    SetNextState(expNextState);
    return true;
}

bool
WriteMpio::_MakeReady(MpAioState expNextState)
{
    currLpn = io.metaLpn;
    currBuf = GetMDPageDataBuf();

    if (IsPartialIO()) // request I/O size != fildDataChunkSize (ex. 4KB)
    {                  // need Modify copyback @ DoIO (read from file + write for request)
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "[Mpio][_MakeReady  ] type={}, req.tagId={}, mpio_id={}, curLpn={}, prevLpn={}, curBufA={}, prevBufA={}",
            io.opcode, io.tagId, io.mpioId, currLpn, prevLpn, currBuf, prevBuf);

        if (IsCacheableVolumeType())
        {
            switch (cacheState)
            {
                case MpioCacheState::Read:
                    // read -> merge data
                    SetNextState(MpAioState::Read);
                    break;

                case MpioCacheState::Merge:
                    // merge data
                    SetNextState(expNextState);
                    break;

                case MpioCacheState::Write:
                    // cached, skip reading & merging, just write
                    SetNextState(MpAioState::PrepareWrite);
                    break;

                default:
                    assert(false);
                    break;
            }
        }
        else
        {
            SetNextState(MpAioState::Read);
        }
    }
    else
    {
        // merge data
        SetNextState(expNextState);
    }

    prevLpn = currLpn;
    prevBuf = currBuf;

    return true;
}

bool
WriteMpio::_Write(const MpAioState expNextState)
{
    StoreTimestamp(MpioTimestampStage::Write);
    return Mpio::DoIO(expNextState);
}

bool
WriteMpio::_PrepareWrite(MpAioState expNextState)
{
    BuildCompositeMDPage();

    // WriteMpio::Write()
    SetNextState(expNextState);

    return true;
}

bool
WriteMpio::_MergeData(MpAioState expNextState)
{
    int contd2NextRun = _MergeMDPage(io.userBuf, io.startByteOffset, io.byteSize, GetMDPageDataBuf());

    // WriteMpio::_PrepareWrite()
    SetNextState(expNextState);

    if (IsCacheableVolumeType() && IsCached())
    {
        ChangeCacheStateTo(MpioCacheState::Write);
    }

    return contd2NextRun;
}

bool
WriteMpio::_MergeMDPage(void* userBuf, FileSizeType userWByteOffset, FileSizeType userWByteSize, void* mdpageBuf)
{
    // copy-back the portion of user data into mdpage
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[Mpio][_MergeData  ] type={}, req.tagId={}, mpio_id={}, fileOffsetinChunk={}, size={}, mpageBufD={}, mpageBufA={}, userbufD={}",
        io.opcode, io.tagId, io.mpioId, userWByteOffset, userWByteSize, *(uint32_t*)mdpageBuf, mdpageBuf, *(uint32_t*)userBuf);

    bool execInSync = _DoMemCpy((uint8_t*)mdpageBuf + userWByteOffset, userBuf, userWByteSize);

    return execInSync;
}

bool
WriteMpio::_HandleError(MpAioState expNextState)
{
    MfsError rc;
    rc = GetErrorStatus();

    if (rc.first != 0)
    {
        MFS_TRACE_ERROR(EID(MFS_MEDIA_WRITE_FAILED),
            "[Mpio][WrMpioError] WriteMpio Error...req.tagId={}, mpio_id={}",
            io.tagId, io.mpioId);
    }

    SetNextState(expNextState);

    return true;
}

bool
WriteMpio::_CompleteIO(MpAioState expNextState)
{
    SetNextState(expNextState);

    mssIntf = nullptr;
    aioModeEnabled = false;

    return true;
}
} // namespace pos
