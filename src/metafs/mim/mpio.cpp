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

#include "mpio.h"

#include "instance_tagid_allocator.h"
#include "meta_storage_specific.h"
#include "mfs_common.h"
#include "mfs_mem_lib.h"

Mpio::Mpio(void* mdPageBuf)
: mdpage(nullptr),
  partialIO(false),
  mssIntf(metaStorage),
  aioModeEnabled(metaStorage->IsAIOSupport()),
  error(0),
  errorStopState(false),
  forceSyncIO(false)
{
    AllocateMDPage(mdPageBuf);
    mpioDoneCallback = AsEntryPointParam1(&Mpio::_HandlePartialDone, this);
}

Mpio::Mpio(void* mdPageBuf, MetaStorageType targetMediaType, MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO)
: io(mpioIoInfo),
  mdpage(nullptr),
  partialIO(partialIO),
  mssIntf(metaStorage),
  aioModeEnabled(metaStorage->IsAIOSupport()),
  error(0),
  errorStopState(false),
  forceSyncIO(forceSyncIO)
{
    AllocateMDPage(mdPageBuf);
    mpioDoneCallback = AsEntryPointParam1(&Mpio::_HandlePartialDone, this);
}

void
Mpio::Reset(void)
{
    // state init.
    MetaAsyncRunnableClass<MetaAsyncCbCxt, MpAioState, MpioStateExecuteEntry>::Init();

    // ctrl. info init.
    mdpage.ClearCtrlInfo();
}

Mpio::~Mpio(void)
{
}

void
Mpio::Setup(MetaStorageType targetMediaType, MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO)
{
    this->io = mpioIoInfo;
    this->partialIO = partialIO;
    this->forceSyncIO = forceSyncIO;
}

void
Mpio::SetLocalAioCbCxt(MpioAsyncDoneCb& callback)
{
    aioCbCxt.Init(this, callback);
    SetAsyncCbCxt(&aioCbCxt, false);
}

uint32_t
Mpio::GetId(void)
{
    return io.mpioId;
}

void
Mpio::SetId(uint32_t id)
{
    io.mpioId = id;
}

bool
Mpio::IsAIOMode(void)
{
    return aioModeEnabled;
}

MfsError
Mpio::GetErrorStatus(void)
{
    MfsError err(this->error, this->errorStopState);
    return err;
}

void
Mpio::SetPartialDoneNotifier(PartialMpioDoneCb& partialMpioDoneNotifier)
{
    this->partialMpioDoneNotifier = partialMpioDoneNotifier;
}

// void
// Mpio::SetTimestampForCompletion(void)
// {
//     time.timeTickCompleted = GetTimestampUs();
// }

bool
Mpio::IsPartialIO(void)
{
    return partialIO;
}

void
Mpio::AllocateMDPage(void* buf)
{
    mdpage.Init(buf);
}

void
Mpio::Issue(void)
{
    assert(GetStateInExecution() == MpAioState::Init);
    ExecuteAsyncState();
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
Mpio::BuildCompositeMDPage(CompMDPageGenClass* compMdPageGenHelper)
{
    mdpage.Make(io.metaLpn, io.targetFD);
    compMdPageGenHelper->CreateCompMDPage(&mdpage);
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

    if ((mdpage.GetmfsSignature() == 0) && (GetOpcode() == MetaIoOpcode::Read))
    {
        // clean page , need to change read state machin.
    }

    bool isValid = mdpage.CheckValid();
    if (!isValid)
    {
        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "it's empty or invalid mdpage");
        return false;
    }
    return true;
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
        assert(false);
    }
#if 0 // MDI implementation

    if (false == mdpage.CheckMetaParity())
    {
        integrityOk = false;
        assert(false);
    }
#endif
    return integrityOk;
}

bool
Mpio::DoE2ECheck(MpAioState expNextState)
{
    if (IsValidPage())
    {
        if (false == CheckDataIntegrity())
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_INVALID_INFORMATION,
                "E2E Check fail!!!!");
            // FIXME: need to handle error
            assert(false);
        }
    }
    else
    {
        // require to memset for invalid page?
        _DoMemSetZero(GetMDPageDataBuf(), mdpage.GetDefaultDataChunkSize());
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
    assert(mssIntf->IsReady());

    bool continueToNextStateRun = true;
    IBOF_EVENT_ID ret;
    void* buf = GetMDPageDataBuf();
    MssOpcode opcode = _ConvertToMssOpcode(GetStateInExecution());

    if (aioModeEnabled && (!forceSyncIO))
    {
        mssAioData.Init(io.targetMediaType, io.metaLpn, io.pageCnt, buf, io.mpioId, io.tagId, io.startByteOffset);
        mssAioCbCxt.Init(&mssAioData, mpioDoneCallback);

        SetNextState(expNextState);

        ret = mssIntf->DoPageIOAsync(opcode, &mssAioCbCxt);

        if (ret != IBOF_EVENT_ID::SUCCESS)
        {
            SetNextState(MpAioState::Error);

            if (ret == IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE)
            {
                errorStopState = true;
            }
        }

        continueToNextStateRun = false;
    }
    else
    {
        ret = mssIntf->DoPageIO(opcode, io.targetMediaType, io.metaLpn, buf, io.pageCnt, io.mpioId, io.tagId);
        if (ret == IBOF_EVENT_ID::SUCCESS)
        {
            SetNextState(expNextState);
        }
        else
        {
            SetNextState(MpAioState::Error);

            if (ret == IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE)
            {
                errorStopState = true;
            }
        }
        _HandlePartialDone();
        continueToNextStateRun = false;
    }

    return continueToNextStateRun;
}

const MetaIoOpcode
Mpio::GetOpcode(void)
{
    return this->io.opcode;
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
    if (MetaFsSysHwMemLib::IsResourceAvailable())
    {
        MetaFsSysHwMemLib::MemCpyAsync(dst, src, nbytes, _HandleAsyncMemOpDone, this);
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
    if (MetaFsSysHwMemLib::IsResourceAvailable())
    {
        MetaFsSysHwMemLib::MemSetZero(addr, nbytes, _HandleAsyncMemOpDone, this);
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
