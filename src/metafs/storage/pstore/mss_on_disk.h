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

#ifndef _INCLUDE_MSS_ONDISK_INCLUDE_H
#define _INCLUDE_MSS_ONDISK_INCLUDE_H

#include <vector>
#include <string>
#include "mfs_asynccb_cxt_template.h"
#include "src/metafs/storage/mss.h"
#include "mss_disk_place.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/event_scheduler/callback.h"
#include "src/include/backend_event.h"

#define INPLACE 1

/**
 * Meta Storage Subsystem Persistant Storage OnDisk Update
 *
 * Provides concrete implementation of API's
 * provided by MetaStorageSubsystem class
 * for Disk OnDisk Store.
 *
 */
namespace pos
{
class MssIoCompletion : public Callback
{
public:
    explicit MssIoCompletion(MssAioCbCxt* cb)
    : Callback(false, CallbackType_MssIoCompletion),
      cbCxt(cb)
    {
        SetEventType(BackendEvent_MetaIO);
    }

    ~MssIoCompletion(void) override
    {
    }

private:
    bool
    _DoSpecificJob(void) override
    {
        int iostatus = _GetErrorCount();

        if (cbCxt != nullptr)
        {
            cbCxt->SaveIOStatus(iostatus);
            cbCxt->InvokeCallback();
        }

        return true;
    }

    MssAioCbCxt* cbCxt;
};

class MssOnDisk : public MetaStorageSubsystem
{
public:
    explicit MssOnDisk(int arrayId);
    virtual ~MssOnDisk(void);

    // Need to remove this function
    virtual POS_EVENT_ID CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag = false) override;
    virtual POS_EVENT_ID Open(void) override;
    virtual POS_EVENT_ID Close(void) override;
    virtual uint64_t GetCapacity(MetaStorageType mediaType) override;
    virtual POS_EVENT_ID ReadPage(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages) override;
    virtual POS_EVENT_ID WritePage(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages) override;
    virtual bool IsAIOSupport(void) override;
    virtual POS_EVENT_ID ReadPageAsync(MssAioCbCxt* cb) override;
    virtual POS_EVENT_ID WritePageAsync(MssAioCbCxt* cb) override;

    virtual POS_EVENT_ID TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages) override;
    virtual LogicalBlkAddr TranslateAddress(MetaStorageType type, MetaLpnType theLpn) override;

private:
    bool _CheckSanityErr(MetaLpnType pageNumber, uint64_t arrayCapacity);
    POS_EVENT_ID _SendSyncRequest(IODirection direction, MetaStorageType mediaType, MetaLpnType pageNumber, MetaLpnType numPages, void* buffer);
    POS_EVENT_ID _SendAsyncRequest(IODirection direction, MssAioCbCxt* cb);
    void _AdjustPageIoToFitTargetPartition(MetaStorageType mediaType, MetaLpnType& targetPage, MetaLpnType& targetNumPages);
    void _Finalize(void);

    std::vector<MssDiskPlace*> mssDiskPlace;
    std::vector<uint64_t> metaCapacity;
    std::vector<uint64_t> totalBlks;
    std::vector<uint32_t> maxPageCntLimitPerIO;

    static const uint32_t MAX_DATA_TRANSFER_BYTE_SIZE = 4 * 1024; // 128 * 1024; temporary changed to 4KB due to FT layer IssueUbio
    uint32_t retryIoCnt;
};
} // namespace pos

#endif // _INCLUDE_MSS_ONDISK_INCLUDE_H
