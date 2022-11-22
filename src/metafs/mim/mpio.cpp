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

#include "meta_storage_specific.h"
#include "metafs_common.h"
#include "metafs_mem_lib.h"
#include "metafs_system_manager.h"

namespace pos
{
std::atomic<uint64_t> Mpio::idAllocate_{0};

Mpio::Mpio(MDPage* mdPage, const bool directAccessEnabled, const bool checkingCrcWhenReading, AsyncCallback callback)
: mdpage(mdPage),
  partialIO(false),
  mssIntf(nullptr),
  aioModeEnabled(false),
  error(0),
  errorStopState(false),
  forceSyncIO(false),
  cacheState(MpioCacheState::Init),
  fileType(MetaFileType::General),
  partialMpioDoneNotifier(nullptr),
  mpioDoneCallback(callback),
  UNIQUE_ID(idAllocate_++),
  DIRECT_ACCESS_ENABLED(directAccessEnabled),
  SUPPORT_CHECKING_CRC_WHEN_READING(checkingCrcWhenReading),
  isAllocated(false)
{
}

Mpio::Mpio(void* mdPageBuf, const bool directAccessEnabled, const bool checkingCrcWhenReading)
: Mpio(new MDPage(mdPageBuf), directAccessEnabled, checkingCrcWhenReading, AsEntryPointParam1(&Mpio::_HandlePartialDone, this))
{
}

void
Mpio::Reset(void)
{
    // state init.
    MetaAsyncRunnable<MetaAsyncCbCxt, MpAioState, MpioStateExecuteEntry>::Init();

    // ctrl. info init.
    mdpage->ClearControlInfo();

    // clear error info
    error = 0;
    errorStopState = false;

    ResetTimestamp();

    _SetAllocated(false);
}

// LCOV_EXCL_START
Mpio::~Mpio(void)
{
    delete mdpage;
}
// LCOV_EXCL_STOP

void
Mpio::Setup(MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO, MetaStorageSubsystem* metaStorage)
{
    _SetAllocated(true);
    this->io = mpioIoInfo;
    this->partialIO = partialIO;
    this->forceSyncIO = forceSyncIO;
    this->mssIntf = metaStorage;
    fileType = mpioIoInfo.fileType;
    aioModeEnabled = metaStorage->IsAIOSupport();
}

void
Mpio::SetLocalAioCbCxt(MpioAsyncDoneCb& callback)
{
    aioCbCxt.Init(this, callback);
    SetAsyncCbCxt(&aioCbCxt, false);
}

void
Mpio::SetPartialDoneNotifier(PartialMpioDoneCb& partialMpioDoneNotifier)
{
    this->partialMpioDoneNotifier = partialMpioDoneNotifier;
}

bool
Mpio::IsPartialIO(void) const
{
    return partialIO;
}

bool
Mpio::CheckReadStatus(const MpAioState expNextState)
{
    return _CheckIOStatus(expNextState);
}

bool
Mpio::CheckWriteStatus(const MpAioState expNextState)
{
    return _CheckIOStatus(expNextState);
}

bool
Mpio::_CheckIOStatus(const MpAioState expNextState)
{
    bool error = false;

    if (aioModeEnabled && (!forceSyncIO))
    {
        if (mssAioData.GetError() != 0 || mssAioData.GetErrorStopState() == true)
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
    mdpage->AttachControlInfo();
    mdpage->BuildControlInfo(io.metaLpn, io.targetFD, io.arrayId, io.signature);
}

void*
Mpio::GetMDPageDataBuf(void) const
{
    return mdpage->GetDataBuffer();
}

void
Mpio::_SetAllocated(const bool flag)
{
    isAllocated = flag;
}

bool
Mpio::_IsAllocated(void) const
{
    return isAllocated;
}

bool
Mpio::DoE2ECheck(const MpAioState expNextState)
{
    bool skipCheckingCrc = false;
    mdpage->AttachControlInfo();
    SetNextState(expNextState);

    if ((DIRECT_ACCESS_ENABLED && MetaStorageType::NVRAM == io.targetMediaType) ||
        (SUPPORT_CHECKING_CRC_WHEN_READING == false))
    {
        skipCheckingCrc = true;
    }

    if (mdpage->IsValidSignature(io.signature))
    {
        if (mdpage->CheckDataIntegrity(io.metaLpn, io.targetFD, skipCheckingCrc))
        {
            SetNextState(MpAioState::Error);
            POS_TRACE_ERROR(EID(MFS_FAILED_TO_CHECK_INTEGRITY),
                "[Mpio][DoE2ECheck ] E2E Check fail!, arrayId={}, mediaType={}, lpn={}",
                io.arrayId, (int)io.targetMediaType, io.metaLpn);
        }
    }
    else
    {
        if (!DIRECT_ACCESS_ENABLED || MetaStorageType::NVRAM != io.targetMediaType)
        {
            MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                "[Mpio][DoE2ECheck ] Read data will be cleared due to invalid data, arrayId={}, mediaType={}, lpn={}, oldData={}",
                io.arrayId, (int)io.targetMediaType, io.metaLpn, *(uint64_t*)GetMDPageDataBuf());

            // require to memset for invalid page?
            _DoMemSetZero();
        }
    }

    return true;
}

MssOpcode
Mpio::_ConvertToMssOpcode(const MpAioState mpioState)
{
    switch (mpioState)
    {
        case MpAioState::Read:
            return MssOpcode::Read;

        case MpAioState::Write:
            return MssOpcode::Write;

        default:
            POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
                "Operation {} is not supported.", (int)mpioState);
            assert(false);
    }
}

bool
Mpio::DoIO(const MpAioState expNextState)
{
    bool continueToNextStateRun = true;
    POS_EVENT_ID ret;
    void* buf = GetMDPageDataBuf();
    MssOpcode opcode = _ConvertToMssOpcode(GetStateInExecution());

    if (IsCacheableVolumeType())
    {
        if (opcode == MssOpcode::Read)
            PrintLog("[io  -   read]", io.arrayId, io.metaLpn);
        else
            PrintLog("[io  -  write]", io.arrayId, io.metaLpn);
    }

    if (aioModeEnabled && (!forceSyncIO))
    {
        FileSizeType startOffset = 0;
        if (!IsCached())
        {
            startOffset = io.startByteOffset;
        }
        mssAioData.Init(io.arrayId, io.targetMediaType, io.metaLpn, io.pageCnt, buf, io.mpioId, io.tagId, startOffset);
        mssAioCbCxt.Init(&mssAioData, mpioDoneCallback);

        SetNextState(expNextState);

        ret = mssIntf->DoPageIOAsync(opcode, &mssAioCbCxt);

        if (ret != EID(SUCCESS))
        {
            SetNextState(MpAioState::Error);

            if (ret == EID(MFS_IO_FAILED_DUE_TO_STOP_STATE))
            {
                errorStopState = true;
                mssAioData.SetErrorStopState(true);
            }
        }

        continueToNextStateRun = false;
    }
    else
    {
        ret = mssIntf->DoPageIO(opcode, io.targetMediaType, io.metaLpn, buf, io.pageCnt, io.mpioId, io.tagId);
        if (ret == EID(SUCCESS))
        {
            SetNextState(expNextState);
        }
        else
        {
            SetNextState(MpAioState::Error);

            if (ret == EID(MFS_IO_FAILED_DUE_TO_STOP_STATE))
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
    if (!_IsAllocated())
    {
        POS_TRACE_ERROR(EID(MFS_MPIO_ENQUEUED_TWICE), "UNIQUE_ID:{}", UNIQUE_ID);
        assert(false);
    }

    // notifiy mpio done to partial mpio done handler (bottomhalf handler of mio handler)
    this->partialMpioDoneNotifier(this); // MpioHandler::EnqueuePartialMpio()
}

bool
Mpio::_DoMemCpy(void* dst, void* src, const size_t nbytes)
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
Mpio::_DoMemSetZero(void)
{
    bool syncOp = true;
    auto addr = GetMDPageDataBuf();
    auto nbytes = mdpage->GetDefaultDataChunkSize();

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
    reinterpret_cast<Mpio*>(obj)->_HandlePartialDone();
}
} // namespace pos
