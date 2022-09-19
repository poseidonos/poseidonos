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

#ifndef _INCLUDE_MSS_ONDISK_INCLUDE_H
#define _INCLUDE_MSS_ONDISK_INCLUDE_H

#include <list>
#include <vector>

#include "meta_async_cb_cxt.h"
#include "mss_disk_place.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/metafs/storage/mss.h"
#include "src/metafs/storage/pstore/mss_io_completion.h"

#define INPLACE 1

namespace pos
{
class MssOnDisk : public MetaStorageSubsystem
{
public:
    MssOnDisk(void) = delete;
    explicit MssOnDisk(const int arrayId);
    virtual ~MssOnDisk(void);

    // Need to remove this function
    virtual POS_EVENT_ID CreateMetaStore(const int arrayId, const MetaStorageType mediaType,
        const uint64_t capacity, const bool formatFlag = false) override;
    virtual POS_EVENT_ID Open(void) override;
    virtual POS_EVENT_ID Close(void) override;
    virtual uint64_t GetCapacity(const MetaStorageType mediaType) override;
    virtual POS_EVENT_ID ReadPage(const MetaStorageType mediaType, const MetaLpnType pageNumber,
        void* buffer, const MetaLpnType numPages) override;
    virtual POS_EVENT_ID WritePage(const MetaStorageType mediaType, const MetaLpnType pageNumber,
        void* buffer, const MetaLpnType numPages) override;
    virtual bool IsAIOSupport(void) override
    {
        return true;
    }
    virtual POS_EVENT_ID ReadPageAsync(MssAioCbCxt* cb) override;
    virtual POS_EVENT_ID WritePageAsync(MssAioCbCxt* cb) override;

    virtual POS_EVENT_ID TrimFileData(const MetaStorageType mediaType, const MetaLpnType startLpn,
        void* buffer, const MetaLpnType numPages) override;
    virtual LogicalBlkAddr TranslateAddress(const MetaStorageType type, const MetaLpnType theLpn) override;

    // for test
    virtual std::vector<MssDiskPlace*>& GetMssDiskPlace(void)
    {
        return mssDiskPlace;
    }

private:
    bool _CheckSanityErr(const MetaLpnType pageNumber, const uint64_t arrayCapacity);
    POS_EVENT_ID _SendSyncRequest(const IODirection direction, const MetaStorageType mediaType,
        const MetaLpnType pageNumber, const MetaLpnType numPages, void* buffer);
    POS_EVENT_ID _SendAsyncRequest(const IODirection direction, MssAioCbCxt* cb);
    void _AdjustPageIoToFitTargetPartition(const MetaStorageType mediaType, MetaLpnType& targetPage,
        MetaLpnType& targetNumPages);
    void _Finalize(void);
    std::list<BufferEntry> _GetBufferList(const MetaStorageType mediaType, const uint64_t offset, const uint64_t count, uint8_t* buffer);

    std::vector<MssDiskPlace*> mssDiskPlace;
    std::vector<uint64_t> metaCapacity;
    std::vector<uint64_t> totalBlks;
    std::vector<uint32_t> maxPageCntLimitPerIO;

    static const uint32_t MAX_DATA_TRANSFER_BYTE_SIZE = 4 * 1024; // 128 * 1024; temporary changed to 4KB due to FT layer IssueUbio
};
} // namespace pos

#endif // _INCLUDE_MSS_ONDISK_INCLUDE_H
