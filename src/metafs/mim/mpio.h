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

#include <atomic>
#include <string>

#include "src/metafs/log/metafs_log.h"
#include "src/include/pos_event_id.h"
#include "mdpage.h"
#include "mfs_async_runnable_template.h"
#include "meta_async_cb_cxt.h"
#include "metafs_def.h"
#include "metafs_io_request.h"
#include "mim_state.h"
#include "mpio_io_info.h"
#include "mpio_state_execute_entry.h"
#include "src/metafs/storage/mss.h"
#include "src/metafs/common/metafs_stopwatch.h"

#define AsMpioStateEntryPoint(funcPointer, obj) AsEntryPointParam1(funcPointer, obj)

namespace pos
{
enum class MpioType
{
    First,
    Write = First,
    Read,
    Last = Read,
    Max,
};

enum class MpioCacheState
{
    Init,   // mpio is not cached
    Read,   // cached, need to read
    Merge,  // cached, need to merge after reading
    Write,  // cached, mergeable, all other data in the meta lpn have been already read
};

class Mpio;
using PartialMpioDoneCb = std::function<void(Mpio*)>;
using MpioAsyncDoneCb = AsyncCallback;

// meta page io class
class Mpio : public MetaAsyncRunnable<MetaAsyncCbCxt, MpAioState, MpioStateExecuteEntry>, public MetaFsStopwatch<MpioTimestampStage>
{
public:
    Mpio(void) = delete;
    explicit Mpio(void* mdPageBuf, const bool directAccessEnabled);
    virtual ~Mpio(void);
    Mpio(const Mpio& mpio) = delete;
    Mpio& operator=(const Mpio& mio) = delete;
    virtual void Reset(void);

    virtual void Setup(MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO, MetaStorageSubsystem* metaStorage);
    virtual void SetLocalAioCbCxt(MpioAsyncDoneCb& callback);
    virtual MpioType GetType(void) const = 0;

    virtual void SetPartialDoneNotifier(PartialMpioDoneCb& partialMpioDoneNotifier);
    virtual bool IsPartialIO(void) const;

    virtual bool IsCacheableVolumeType(void) const
    {
        return (MetaStorageType::SSD != io.targetMediaType);
    }
    virtual bool IsCached(void) const
    {
        return (MpioCacheState::Init != cacheState);
    }
    virtual bool IsMergeable(void) const
    {
        return (MpioCacheState::Write == cacheState);
    }
    virtual void ChangeCacheStateTo(const MpioCacheState state)
    {
        cacheState = state;
    }

    virtual void BuildCompositeMDPage(void);
    virtual void* GetMDPageDataBuf(void) const;

    virtual bool DoIO(const MpAioState expNextState);
    virtual bool DoE2ECheck(const MpAioState expNextState);
    virtual bool CheckReadStatus(const MpAioState expNextState);
    virtual bool CheckWriteStatus(const MpAioState expNextState);
    virtual MfsError GetErrorStatus(void) const
    {
        return {error, errorStopState};
    }
    virtual uint64_t GetId(void) const
    {
        return UNIQUE_ID;
    }
    virtual MetaFileType GetFileType(void) const
    {
        return fileType;
    }
    void PrintLog(const std::string& str, const int array, const int lpn) const
    {
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            str + " id: {}, array: {}, lpn: {}", GetId(), array, lpn);
    }

    MpioIoInfo io;

protected:
    MDPage mdpage;

    bool partialIO;
    MetaStorageSubsystem* mssIntf;
    bool aioModeEnabled;
    int error;
    bool errorStopState;
    bool forceSyncIO;
    MetaAsyncCbCxt aioCbCxt;

    MpioCacheState cacheState;
    MetaFileType fileType;

    virtual void _InitStateHandler(void) = 0;
    bool _DoMemCpy(void* dst, void* src, const size_t nbytes);
    bool _DoMemSetZero(void* addr, const size_t nbytes);

    static void _HandleAsyncMemOpDone(void* obj);
    void _HandlePartialDone(void* notused = nullptr);

    bool _CheckDataIntegrity(void) const;

private:
    MssOpcode _ConvertToMssOpcode(const MpAioState mpioState);
    bool _CheckIOStatus(const MpAioState expNextState);
    void _BackupMssAioCbCxtPointer(MssAioCbCxt* cbCxt);

    MssAioData mssAioData;
    MssAioCbCxt mssAioCbCxt;

    PartialMpioDoneCb partialMpioDoneNotifier;
    AsyncCallback mpioDoneCallback;

    const uint64_t UNIQUE_ID;
    static std::atomic<uint64_t> idAllocate_;
    const bool DIRECT_ACCESS_ENABLED;
};
} // namespace pos
