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

#include "src/metafs/include/meta_storage_info.h"
#include "src/metafs/mai/metafs_file_control_api.h"
#include "src/metafs/mai/metafs_management_api.h"
#include "src/metafs/mai/metafs_io_api.h"
#include "src/metafs/mai/metafs_wbt_api.h"
#include "src/metafs/storage/mss.h"

#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/address_type.h"

namespace pos
{
class MetaFs : public IMountSequence
{
public:
    MetaFs(void);
    MetaFs(IArrayInfo* arrayInfo, bool isLoaded);
    MetaFs(IArrayInfo* arrayInfo, bool isLoaded, MetaFsManagementApi* mgmt,
            MetaFsFileControlApi* ctrl, MetaFsIoApi* io, MetaFsWBTApi* wbt,
            MetaStorageSubsystem* metaStorage, TelemetryPublisher* tp);
    virtual ~MetaFs(void);

    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;

    virtual uint64_t GetEpochSignature(void);
    virtual StripeId GetTheLastValidStripeId(void);
    MetaStorageSubsystem* GetMss(void);
    virtual int EstimateAlignedFileIOSize(MetaFilePropertySet& prop);

    MetaFsManagementApi* mgmt = nullptr;
    MetaFsIoApi* io = nullptr;
    MetaFsFileControlApi* ctrl = nullptr;
    MetaFsWBTApi* wbt = nullptr;

private:
    bool _Initialize(void);
    POS_EVENT_ID _PrepareMetaVolume(void);
    POS_EVENT_ID _CreateMetaVolume(void);
    POS_EVENT_ID _OpenMetaVolume(void);
    POS_EVENT_ID _CloseMetaVolume(void);

    void _RegisterMediaInfoIfAvailable(PartitionType ptnType, MetaStorageInfoList& mediaList);
    std::shared_ptr<MetaStorageInfo> _MakeMetaStorageMediaInfo(PartitionType ptnType);

    bool isNpor;
    bool isLoaded;
    bool isNormal;
    IArrayInfo* arrayInfo;
    std::string arrayName;
    int arrayId;
    MetaStorageSubsystem* metaStorage;
    TelemetryPublisher* telemetryPublisher;
};
} // namespace pos
