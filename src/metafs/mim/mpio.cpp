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

#include "mpio.h"

#include "Air.h"
#include "meta_storage_specific.h"
#include "metafs_common.h"
#include "metafs_mem_lib.h"
#include "metafs_system_manager.h"

namespace pos
{
std::atomic<uint64_t> Mpio::idAllocate_{0};

Mpio::Mpio(void* mdPageBuf, const bool directAccessEnabled)
: mdpage(mdPageBuf),
  partialIO(false),
  mssIntf(nullptr),
  aioModeEnabled(false),
  error(0),
  errorStopState(false),
  forceSyncIO(false),
  cacheState(MpioCacheState::Init),
  priority(RequestPriority::Normal),
  UNIQUE_ID(idAllocate_++),
  DIRECT_ACCESS_ENABLED(directAccessEnabled)
{
    mpioDoneCallback = AsEntryPointParam1(&Mpio::_HandlePartialDone, this);
}

void
Mpio::Reset(void)
{
    // state init.
    MetaAsyncRunnable<MetaAsyncCbCxt, MpAioState, MpioStateExecuteEntry>::Init();

    // ctrl. info init.
    mdpage.ClearCtrlInfo();

    // clear error info
    error = 0;
    errorStopState = false;

    ResetTimestamp();
}

// LCOV_EXCL_START
Mpio::~Mpio(void)
{
}
// LCOV_EXCL_STOP

void
Mpio::Setup(MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO, MetaStorageSubsystem* metaStorage)
{
    this->io = mpioIoInfo;
    this->partialIO = partialIO;
    this->forceSyncIO = forceSyncIO;
    this->mssIntf = metaStorage;
    aioModeEnabled = metaStorage->IsAIOSupport();
}

void
Mpio::SetLocalAioCbCxt(MpioAsyncDoneCb& callback)
{
    aioCbCxt.Init(this, callback);
    SetAsyncCbCxt(&aioCbCxt, false);
}

MfsError
Mpio::GetErrorStatus(void)
{
    MfsError err(this->error, this->errorStopState);
    return err;
}

void
Mpio::SetPriority(RequestPriority p)
{
    priority = p;
}

RequestPriority
Mpio::GetPriority(void)
{
    return priority;
}

void
Mpio::SetPartialDoneNotifier(PartialMpioDoneCb& partialMpioDoneNotifier)
{
    this->partialMpioDoneNotifier = partialMpioDoneNotifier;
}

bool
Mpio::IsPartialIO(void)
{
    return partialIO;
}

MpioCacheState
Mpio::GetCacheState(void)
{
    return cacheState;
}

void
Mpio::SetCacheState(MpioCacheState state)
{
    cacheState = state;
}

bool
Mpio::CheckReadStatus(MpAioState expNextState)
{
    return _CheckIOStatus(expNextState);
}

bool
Mpio::CheckWriteStatus(MpAioState expNextState)
{
    return _CheckIOStatus(expNextState);
}

bool
Mpio::_CheckIOStatus(MpAioState expNextState)
{
    bool error = false;

    if (aioModeEnabled && (!forceSyncIO))
    {
        if (mssAioData.error != 0 || mssAioData.errorStopState == true)
        {
            error = true;
        }
    }

    if (error)
    {
        SetNextState(MpAioState::Error);
    }
    else
    {
        SetNextState(expNextState);
    }
    return true;
}

void
Mpio::BuildCompositeMDPage(void)
{
    mdpage.Make(io.metaLpn, io.targetFD, io.arrayId);
}

void*
Mpio::GetMDPageDataBuf(void)
{
    return mdpage.GetDataBuf();
}

void*
Mpio::GetUserDataBuf(void)
{
    return io.userBuf;
}

bool
Mpio::IsValidPage(void)
{
    mdpage.AttachControlInfo();

    return mdpage.CheckValid(io.arrayId);
}

bool
Mpio::CheckDataIntegrity(void)
{
    mdpage.AttachControlInfo();

    bool integrityOk = true;
    if (false == mdpage.CheckLpnMismatch(io.metaLpn) ||
        false == mdpage.CheckFileMismatch(io.targetFD))
    {
        integrityOk = false;
    }

    return integrityOk;
}

bool
Mpio::DoE2ECheck(MpAioState expNextState)
{
    if (IsValidPage())
    {
        if (false == CheckDataIntegrity())
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_INFORMATION,
                "[Mpio][DoE2ECheck ] E2E Check fail!, arrayId={}, mediaType={}, lpn={}",
                io.arrayId, (int)io.targetMediaType, io.metaLpn);

            // FIXME: need to handle error
            assert(false);
        }
    }
    else
    {
        if (!DIRECT_ACCESS_ENABLED || MetaStorageType::NVRAM != io.targetMediaType)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[Mpio][DoE2ECheck ] Read data will be cleared due to invalid data, arrayId={}, mediaType={}, lpn={}, oldData={}",
                io.arrayId, (int)io.targetMediaType, io.metaLpn, *(uint64_t*)GetMDPageDataBuf());

            // require to memset for invalid page?
            _DoMemSetZero(GetMDPageDataBuf(), mdpage.GetDefaultDataChunkSize());
        }
    }
    SetNextState(expNextState);

    return true;
}

MssOpcode
Mpio::_ConvertToMssOpcode(const MpAioState mpioState)
{
    if (mpioState == MpAioState::Read)
    {
        return MssOpcode::Read;
    }
    else if (mpioState == MpAioState::Write)
    {
        return MssOpcode::Write;
    }
    else
    {
        assert(false);
    }
}

bool
Mpio::DoIO(MpAioState expNextState)
{
    bool continueToNextStateRun = true;
    POS_EVENT_ID ret;
    void* buf = GetMDPageDataBuf();
    MssOpcode opcode = _ConvertToMssOpcode(GetStateInExecution());

    if (io.targetMediaType == MetaStorageType::NVRAM)
    {
        if (opcode == MssOpcode::Read)
            PrintLog("[io  -   read]", io.arrayId, io.metaLpn);
        else
            PrintLog("[io  -  write]", io.arrayId, io.metaLpn);
    }

    if (aioModeEnabled && (!forceSyncIO))
    {
        if (cacheState != MpioCacheState::Init)
        {
            mssAioData.Init(io.arrayId, io.targetMediaType, io.metaLpn, io.pageCnt, buf, io.mpioId, io.tagId, 0);
        }
        else
        {
            mssAioData.Init(io.arrayId, io.targetMediaType, io.metaLpn, io.pageCnt, buf, io.mpioId, io.tagId, io.startByteOffset);
        }
        mssAioCbCxt.Init(&mssAioData, mpioDoneCallback);

        SetNextState(expNextState);

        ret = mssIntf->DoPageIOAsync(opcode, &mssAioCbCxt);

        if (ret != POS_EVENT_ID::SUCCESS)
        {
            SetNextState(MpAioState::Error);

            if (ret == POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE)
            {
                errorStopState = true;
                mssAioData.errorStopState = true;
            }
        }

        continueToNextStateRun = false;
    }
    else
    {
        ret = mssIntf->DoPageIO(opcode, io.targetMediaType, io.metaLpn, buf, io.pageCnt, io.mpioId, io.tagId);
        if (ret == POS_EVENT_ID::SUCCESS)
        {
            SetNextState(expNextState);
        }
        else
        {
            SetNextState(MpAioState::Error);

            if (ret == POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE)
            {
                errorStopState = true;
            }
        }
        _HandlePartialDone();
        continueToNextStateRun = false;
    }

    return continueToNextStateRun;
}

// a function is used to std::function<void(void*)> to std::function<void(Mpio*)>
void
Mpio::_HandlePartialDone(void* notused)
{
    // notifiy mpio done to partial mpio done handler (bottomhalf handler of mio handler)
    this->partialMpioDoneNotifier(this); // MpioHandler::EnqueuePartialMpio()
}

bool
Mpio::_DoMemCpy(void* dst, void* src, size_t nbytes)
{
    bool syncOp = true;
    if (MetaFsMemLib::IsResourceAvailable())
    {
        MetaFsMemLib::MemCpyAsync(dst, src, nbytes, _HandleAsyncMemOpDone, this);
        syncOp = false;
    }
    else
    {
        memcpy(dst, src, nbytes);
    }
    return syncOp;
}

bool
Mpio::_DoMemSetZero(void* addr, size_t nbytes)
{
    bool syncOp = true;
    if (MetaFsMemLib::IsResourceAvailable())
    {
        MetaFsMemLib::MemSetZero(addr, nbytes, _HandleAsyncMemOpDone, this);
        syncOp = false;
    }
    else
    {
        memset(addr, 0x0, nbytes);
    }
    return syncOp;
}

void
Mpio::_HandleAsyncMemOpDone(void* obj)
{
    Mpio* mpio = reinterpret_cast<Mpio*>(obj);
    mpio->_HandlePartialDone();
}
} // namespace pos
