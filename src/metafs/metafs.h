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

#include <memory>
#include <string>

#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/include/address_type.h"
#include "src/metafs/include/meta_storage_info.h"
#include "src/metafs/lib/concurrent_metafs_time_interval.h"
#include "src/metafs/mai/metafs_file_control_api.h"
#include "src/metafs/mai/metafs_io_api.h"
#include "src/metafs/mai/metafs_management_api.h"
#include "src/metafs/mai/metafs_wbt_api.h"
#include "src/metafs/storage/mss.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/metafs/mvm/volume/file_descriptor_allocator.h"
#include "rocksdb/db.h"

namespace pos
{
class MetaFsConfigManager;

class MetaFs : public IMountSequence
{
public:
    MetaFs(void);
    MetaFs(IArrayInfo* arrayInfo, const bool isLoaded);
    // for test
    MetaFs(IArrayInfo* arrayInfo, const bool isLoaded, MetaFsManagementApi* mgmt,
        MetaFsFileControlApi* ctrl, MetaFsIoApi* io, MetaFsWBTApi* wbt,
        MetaStorageSubsystem* metaStorage, TelemetryPublisher* tp);
    virtual ~MetaFs(void);

    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;

    virtual uint64_t GetEpochSignature(void);
    virtual StripeId GetTheLastValidStripeId(void);

    virtual rocksdb::DB* GetRocksMeta(void)
    {
        return rocksMeta;
    }
    virtual FileDescriptorAllocator* GetFileDescriptorAllocator(void)
    {
        return fileDescriptorAllocator;
    }
    virtual int EstimateAlignedFileIOSize(MetaFilePropertySet& prop);

    MetaFsManagementApi* mgmt;
    MetaFsIoApi* io;
    MetaFsFileControlApi* ctrl;
    MetaFsWBTApi* wbt;

private:
    bool _Initialize(void);
    POS_EVENT_ID _PrepareMetaVolume(void);
    POS_EVENT_ID _CreateMetaVolume(void);
    POS_EVENT_ID _OpenMetaVolume(void);
    POS_EVENT_ID _CloseMetaVolume(void);
    POS_EVENT_ID _CreateRocksDBMetaFs(void);

    void _RegisterMediaInfoIfAvailable(PartitionType ptnType, MetaStorageInfoList& mediaList);
    std::shared_ptr<MetaStorageInfo> _MakeMetaStorageMediaInfo(PartitionType ptnType);
    MaxMetaLpnMapPerMetaStorage _MakeLpnMap(void) const;

    ConcurrentMetaFsTimeInterval* concurrentMetaFsTimeInterval;
    bool isNpor_;
    bool isLoaded_;
    bool isNormal_;
    IArrayInfo* arrayInfo_;
    std::string arrayName_;
    int arrayId_;
    MetaStorageSubsystem* metaStorage_;
    TelemetryPublisher* telemetryPublisher_;
    rocksdb::DB* rocksMeta;
    FileDescriptorAllocator* fileDescriptorAllocator;
    MetaFsConfigManager* configMgr_;
};
} // namespace pos
