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

#include "metafs.h"

#include <string>

#include "src/include/array_config.h"
#include "src/include/partition_type.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/log/metafs_log.h"
#include "src/metafs/storage/pstore/mss_on_disk.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
MetaFs::MetaFs(void)
: isNpor(false),
  isLoaded(false),
  isNormal(false),
  arrayInfo(nullptr),
  arrayName(""),
  arrayId(INT32_MAX),
  metaStorage(nullptr),
  telemetryPublisher(nullptr)
{
}

MetaFs::MetaFs(IArrayInfo* arrayInfo, bool isLoaded)
: MetaFs()
{
    this->isLoaded = isLoaded;
    this->arrayInfo = arrayInfo;

    arrayName = arrayInfo->GetName();
    arrayId = arrayInfo->GetIndex();

    metaStorage = new MssOnDisk(arrayId);

    telemetryPublisher = new TelemetryPublisher("metafs_" + to_string(arrayId));
    TelemetryClientSingleton::Instance()->RegisterPublisher(telemetryPublisher);

    mgmt = new MetaFsManagementApi(arrayId, metaStorage);
    ctrl = new MetaFsFileControlApi(arrayId, metaStorage);
    io = new MetaFsIoApi(arrayId, ctrl, metaStorage, telemetryPublisher);
    wbt = new MetaFsWBTApi(arrayId, ctrl);

    MetaFsServiceSingleton::Instance()->Register(arrayName, arrayId, this);
}

MetaFs::MetaFs(IArrayInfo* arrayInfo, bool isLoaded, MetaFsManagementApi* mgmt,
    MetaFsFileControlApi* ctrl, MetaFsIoApi* io, MetaFsWBTApi* wbt,
    MetaStorageSubsystem* metaStorage, TelemetryPublisher* tp)
: MetaFs()
{
    this->isLoaded = isLoaded;
    this->arrayInfo = arrayInfo;

    arrayName = arrayInfo->GetName();
    arrayId = arrayInfo->GetIndex();

    this->mgmt = mgmt;
    this->ctrl = ctrl;
    this->io = io;
    this->wbt = wbt;
    this->metaStorage = metaStorage;

    telemetryPublisher = tp;

    if (nullptr != telemetryPublisher)
        TelemetryClientSingleton::Instance()->RegisterPublisher(telemetryPublisher);

    MetaFsServiceSingleton::Instance()->Register(arrayName, arrayId, this);
}

MetaFs::~MetaFs(void)
{
    MetaFsServiceSingleton::Instance()->Deregister(arrayName);

    if (nullptr != telemetryPublisher)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(telemetryPublisher->GetName());
        delete telemetryPublisher;
        telemetryPublisher = nullptr;
    }

    if (nullptr != mgmt)
        delete mgmt;

    if (nullptr != io)
        delete io;

    if (nullptr != ctrl)
        delete ctrl;

    if (nullptr != wbt)
        delete wbt;

    if (nullptr != metaStorage)
    {
        metaStorage->Close();
        delete metaStorage;
        metaStorage = nullptr;
    }
}

int
MetaFs::Init(void)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;

    if (false == _Initialize())
        return (int)POS_EVENT_ID::MFS_MODULE_INIT_FAILED;

    rc = _PrepareMetaVolume();
    if (POS_EVENT_ID::SUCCESS != rc)
        return (int)rc;

    // MetaFsSystemState::Create
    if (!isLoaded)
    {
        rc = _CreateMetaVolume();
        if (POS_EVENT_ID::SUCCESS != rc)
            return (int)rc;

        isLoaded = true;
    }

    rc = _OpenMetaVolume();
    if (POS_EVENT_ID::SUCCESS != rc)
        return (int)rc;

    if (false == io->AddArray(arrayId))
        return (int)POS_EVENT_ID::MFS_ARRAY_ADD_FAILED;

    isNormal = true;
    mgmt->SetStatus(isNormal);
    io->SetStatus(isNormal);
    ctrl->SetStatus(isNormal);
    wbt->SetStatus(isNormal);

    return (int)POS_EVENT_ID::SUCCESS;
}

void
MetaFs::Dispose(void)
{
    POS_EVENT_ID rc = _CloseMetaVolume();
    if (rc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "It's failed to close meta volume, arrayName={}", arrayName);
    }

    // the storage will be close in mgmt mgr
    rc = mgmt->CloseSystem(arrayId);
    if (rc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_WARN((int)rc,
            "It's failed to unmount system, arrayName={}", arrayName);
    }

    io->RemoveArray(arrayId);
}

void
MetaFs::Shutdown(void)
{
    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Shutdown metafs, arrayName={}", arrayName);

    io->RemoveArray(arrayId);
}

void
MetaFs::Flush(void)
{
    // no-op for IMountSequence
}

uint64_t
MetaFs::GetEpochSignature(void)
{
    return mgmt->GetEpochSignature();
}

StripeId
MetaFs::GetTheLastValidStripeId(void)
{
    if ((nullptr == metaStorage) || (nullptr == ctrl))
        return 0;

    MetaLpnType theLastLpn = ctrl->GetTheLastValidLpn(MetaVolumeType::SsdVolume);
    LogicalBlkAddr addr = metaStorage->TranslateAddress(MetaStorageType::SSD, theLastLpn);

    return addr.stripeId;
}

MetaStorageSubsystem*
MetaFs::GetMss(void)
{
    return metaStorage;
}

int
MetaFs::EstimateAlignedFileIOSize(MetaFilePropertySet& prop)
{
    return ctrl->EstimateAlignedFileIOSize(prop);
}

bool
MetaFs::_Initialize(void)
{
    MetaStorageInfoList infoList;
    _RegisterMediaInfoIfAvailable(META_NVM, infoList);
    _RegisterMediaInfoIfAvailable(META_SSD, infoList);
    _RegisterMediaInfoIfAvailable(JOURNAL_SSD, infoList);

    if (infoList.empty())
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_MODULE_NO_MEDIA,
            "No registered media info detected.");

        return false;
    }

    if (POS_EVENT_ID::SUCCESS != mgmt->InitializeSystem(arrayId, &infoList))
        return false;

    return true;
}

POS_EVENT_ID
MetaFs::_PrepareMetaVolume(void)
{
    // MetaFsSystemState::PowerOn
    // nothing

    // MetaFsSystemState::Init
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsStorageIoInfoList& mediaInfoList = mgmt->GetAllStoragePartitionInfo();
    for (auto& item : mediaInfoList)
    {
        if (false == item.valid)
            continue;

        MetaVolumeType volumeType = MetaFileUtil::ConvertToVolumeType(item.mediaType);
        MetaLpnType maxVolumeLpn = item.totalCapacity / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;

        if (MetaStorageType::SSD == item.mediaType)
        {
            maxVolumeLpn -= mgmt->GetRegionSizeInLpn(); // considered due to MBR placement for SSD volume
        }

        ctrl->InitVolume(volumeType, arrayId, maxVolumeLpn);

        rc = metaStorage->CreateMetaStore(arrayId, item.mediaType, item.totalCapacity, !isLoaded);
        if (rc != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
                "Failed to create meta storage subsystem");
            return POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
        }
    }

    return rc;
}

POS_EVENT_ID
MetaFs::_CreateMetaVolume(void)
{
    MetaFsStorageIoInfoList& mediaInfoList = mgmt->GetAllStoragePartitionInfo();

    for (auto& item : mediaInfoList)
    {
        if (false == item.valid)
            continue;

        MetaVolumeType volumeType = MetaFileUtil::ConvertToVolumeType(item.mediaType);

        if (false == ctrl->CreateVolume(volumeType))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
                "Error occurred to create volume (volume id={})",
                (int)volumeType);

            return POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED;
        }
    }

    if (true != mgmt->CreateMbr())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
            "Error occurred to create MetaFs MBR");

        return POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED;
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaFs::_OpenMetaVolume(void)
{
    // MetaFsSystemState::Open
    POS_EVENT_ID rc = mgmt->LoadMbr(isNpor);

    POSMetric metric(TEL40000_METAFS_NORMAL_SHUTDOWN, POSMetricTypes::MT_GAUGE);
    metric.AddLabel("array_id", to_string(arrayId));
    metric.SetGaugeValue((int)isNpor);
    telemetryPublisher->PublishMetric(metric);

    if (rc != POS_EVENT_ID::SUCCESS)
    {
        if (true == mgmt->IsMbrClean())
        {
            MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "Mbr is clean. This array is mounted for the first time");

            if (false == _Initialize())
                return POS_EVENT_ID::MFS_MODULE_INIT_FAILED;

            rc = _CreateMetaVolume();

            if (rc != POS_EVENT_ID::SUCCESS)
            {
                return rc;
            }
        }
        else
        {
            return rc;
        }
    }

    if (false == ctrl->OpenVolume(isNpor))
    {
        return POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED;
    }

    // MetaFsSystemState::Active
    // nothing

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaFs::_CloseMetaVolume(void)
{
    // MetaFsSystemState::Quiesce
    // nothing

    // MetaFsSystemState::Shutdown
    bool resetCxt = false;
    if (!ctrl->CloseVolume(resetCxt))
    {
        // Reset MetaFS DRAM Context
        if (resetCxt == true)
            return POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED;
        else
            return POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE;
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Meta filesystem has been unmounted...");

    return POS_EVENT_ID::SUCCESS;
}

void
MetaFs::_RegisterMediaInfoIfAvailable(PartitionType ptnType, MetaStorageInfoList& mediaList)
{
    std::shared_ptr<MetaStorageInfo> media = _MakeMetaStorageMediaInfo(ptnType);
    if (!media->valid)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "PartitionType {} is not available.", (int)ptnType);
    }
    mediaList.push_back(media);
}

std::shared_ptr<MetaStorageInfo>
MetaFs::_MakeMetaStorageMediaInfo(PartitionType ptnType)
{
    const PartitionLogicalSize* ptnSize = arrayInfo->GetSizeInfo(ptnType);

    std::shared_ptr<MetaStorageInfo> newInfo = std::make_shared<MetaStorageInfo>();
    switch (ptnType)
    {
        case META_NVM:
            newInfo->media = MetaStorageType::NVRAM;
            break;

        case JOURNAL_SSD:
            newInfo->media = MetaStorageType::JOURNAL_SSD;
            break;

        default:
            newInfo->media = MetaStorageType::SSD;
            break;
    }

    if (ptnSize)
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "ptnType: {}, totalStripes: {}", (int)ptnType, ptnSize->totalStripes);
        newInfo->valid = true;
        newInfo->mediaCapacity = static_cast<uint64_t>(ptnSize->totalStripes) *
            ptnSize->blksPerStripe * ArrayConfig::BLOCK_SIZE_BYTE;
    }

    return newInfo;
}
} // namespace pos
