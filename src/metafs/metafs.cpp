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
#include <experimental/filesystem>

#include "src/include/array_config.h"
#include "src/include/partition_type.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/log/metafs_log.h"
#include "src/metafs/storage/pstore/mss_on_disk.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/metafs/config/metafs_config_manager.h"

namespace pos
{
MetaFs::MetaFs(void)
: mgmt(nullptr),
  io(nullptr),
  ctrl(nullptr),
  wbt(nullptr),
  concurrentMetaFsTimeInterval(nullptr),
  isNpor_(false),
  isLoaded_(false),
  isNormal_(false),
  arrayInfo_(nullptr),
  arrayName_(""),
  arrayId_(INT32_MAX),
  metaStorage_(nullptr),
  telemetryPublisher_(nullptr),
  rocksMeta(nullptr),
  fileDescriptorAllocator(nullptr),
  configMgr_(MetaFsServiceSingleton::Instance()->GetConfigManager())
{
}

MetaFs::MetaFs(IArrayInfo* arrayInfo, const bool isLoaded)
: MetaFs()
{
    arrayInfo_ = arrayInfo;
    isLoaded_ = isLoaded;

    arrayName_ = arrayInfo->GetName();
    arrayId_ = arrayInfo->GetIndex();

    metaStorage_ = new MssOnDisk(arrayId_);

    telemetryPublisher_ = new TelemetryPublisher("metafs_" + to_string(arrayId_));
    TelemetryClientSingleton::Instance()->RegisterPublisher(telemetryPublisher_);

    // TODO(sang7.park) : need to combine with metafs config manager
    concurrentMetaFsTimeInterval = new ConcurrentMetaFsTimeInterval(configMgr_->GetTimeIntervalInMillisecondsForMetric());
    bool supportNuma = configMgr_->IsSupportingNumaDedicatedScheduling();

    mgmt = new MetaFsManagementApi(arrayId_, metaStorage_);
    ctrl = new MetaFsFileControlApi(arrayId_, metaStorage_, mgmt, telemetryPublisher_);
    io = new MetaFsIoApi(arrayId_, ctrl, metaStorage_, telemetryPublisher_, concurrentMetaFsTimeInterval, supportNuma);
    wbt = new MetaFsWBTApi(arrayId_, ctrl);

    MetaFsServiceSingleton::Instance()->Register(arrayName_, arrayId_, this);
}

MetaFs::MetaFs(IArrayInfo* arrayInfo, const bool isLoaded, MetaFsManagementApi* mgmt,
    MetaFsFileControlApi* ctrl, MetaFsIoApi* io, MetaFsWBTApi* wbt,
    MetaStorageSubsystem* metaStorage_, TelemetryPublisher* tp)
: mgmt(mgmt),
  io(io),
  ctrl(ctrl),
  wbt(wbt),
  isNpor_(false),
  isLoaded_(isLoaded),
  isNormal_(false),
  arrayInfo_(arrayInfo),
  arrayName_(arrayInfo->GetName()),
  arrayId_(arrayInfo->GetIndex()),
  metaStorage_(metaStorage_),
  telemetryPublisher_(tp),
  rocksMeta(nullptr),
  fileDescriptorAllocator(nullptr)
{
    if (nullptr != telemetryPublisher_)
        TelemetryClientSingleton::Instance()->RegisterPublisher(telemetryPublisher_);

    MetaFsServiceSingleton::Instance()->Register(arrayName_, arrayId_, this);
}

MetaFs::~MetaFs(void)
{
    MetaFsServiceSingleton::Instance()->Deregister(arrayName_);

    if (nullptr != telemetryPublisher_)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(telemetryPublisher_->GetName());
        delete telemetryPublisher_;
    }

    if (nullptr != mgmt)
    {
        delete mgmt;
        mgmt = nullptr;
    }

    if (nullptr != io)
    {
        delete io;
        io = nullptr;
    }

    if (nullptr != ctrl)
    {
        delete ctrl;
        ctrl = nullptr;
    }

    if (nullptr != wbt)
    {
        delete wbt;
        wbt = nullptr;
    }

    if (nullptr != metaStorage_)
    {
        metaStorage_->Close();
        delete metaStorage_;
        metaStorage_ = nullptr;
    }

    if (nullptr != rocksMeta)
    {
        delete rocksMeta;
        rocksMeta = nullptr;
    }

    if (nullptr != fileDescriptorAllocator)
    {
        delete fileDescriptorAllocator;
        fileDescriptorAllocator = nullptr;
    }
}

int
MetaFs::Init(void)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    if (!_Initialize())
        return EID(MFS_MODULE_INIT_FAILED);

    rc = _PrepareMetaVolume();
    if (EID(SUCCESS) != rc)
        return -(int)rc;

    if (!isLoaded_)
    {
        rc = _CreateMetaVolume();
        if (EID(SUCCESS) != rc)
            return (int)rc;

        isLoaded_ = true;
    }

    rc = _OpenMetaVolume();
    if (EID(SUCCESS) != rc)
        return -(int)rc;

    if (!io->AddArray(arrayId_, _MakeLpnMap()))
        return ERRID(MFS_ARRAY_ADD_FAILED);
    if (MetaFsServiceSingleton::Instance()->GetConfigManager()->IsRocksdbEnabled())
    {
        rc = _CreateRocksDBMetaFs();
        if (EID(SUCCESS) != rc)
            return -(int)rc;
    }

    isNormal_ = true;
    mgmt->SetStatus(isNormal_);
    io->SetStatus(isNormal_);
    ctrl->SetStatus(isNormal_);
    wbt->SetStatus(isNormal_);

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Mount metafs, arrayId: {}", arrayId_);

    return EID(SUCCESS);
}

MaxMetaLpnMapPerMetaStorage
MetaFs::_MakeLpnMap(void) const
{
    MaxMetaLpnMapPerMetaStorage map;

    for (uint32_t idx = 0; idx < (uint32_t)MetaVolumeType::Max; ++idx)
    {
        MetaVolumeType storage = static_cast<MetaVolumeType>(idx);
        if (mgmt->IsValidVolume(storage))
        {
            map.insert({MetaFileUtil::ConvertToMediaType(storage), ctrl->GetMaxMetaLpn(storage)});
        }
    }

    return map;
}

void
MetaFs::Dispose(void)
{
    POS_EVENT_ID rc = _CloseMetaVolume();
    if (rc != EID(SUCCESS))
    {
        POS_TRACE_WARN(EID(MFS_META_VOLUME_CLOSE_FAILED),
            "It's failed to close meta volume, arrayId: {}", arrayId_);
    }

    // the storage will be close in mgmt mgr
    rc = mgmt->CloseSystem(arrayId_);
    if (rc != EID(SUCCESS))
    {
        POS_TRACE_WARN((int)rc,
            "It's failed to unmount system, arrayId: {}", arrayId_);
    }

    if (!io->RemoveArray(arrayId_))
    {
        POS_TRACE_WARN(EID(MFS_INFO_MESSAGE),
            "It's failed to remove array, arrayId: {}", arrayId_);
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Unmount metafs, arrayId: {}", arrayId_);
}

void
MetaFs::Shutdown(void)
{
    if (!io->RemoveArray(arrayId_))
    {
        POS_TRACE_WARN(EID(MFS_INFO_MESSAGE),
            "It's failed to remove array, arrayId: {}", arrayId_);
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Shutdown metafs, arrayId: {}", arrayId_);
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
    if (!metaStorage_)
    {
        POS_TRACE_WARN(EID(MFS_INFO_MESSAGE),
            "Meta storage is not valid, arrayId: {}", arrayId_);
        return 0;
    }
    else if (!ctrl)
    {
        POS_TRACE_WARN(EID(MFS_INFO_MESSAGE),
            "Meta control api is not valid, arrayId: {}", arrayId_);
        return 0;
    }

    MetaLpnType theLastLpn = ctrl->GetTheLastValidLpn(MetaVolumeType::SsdVolume);
    LogicalBlkAddr addr = metaStorage_->TranslateAddress(MetaStorageType::SSD, theLastLpn);

    return addr.stripeId;
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
        POS_TRACE_WARN(EID(MFS_MODULE_NO_MEDIA),
            "No registered media info detected.");

        return false;
    }
    else
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "{} entries have been registered to MediaInfo", infoList.size());
    }

    if (EID(SUCCESS) != mgmt->InitializeSystem(arrayId_, &infoList))
        return false;
    return true;
}

POS_EVENT_ID
MetaFs::_PrepareMetaVolume(void)
{
    POS_EVENT_ID rc = EID(SUCCESS);
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

        ctrl->InitVolume(volumeType, arrayId_, maxVolumeLpn);

        rc = metaStorage_->CreateMetaStore(arrayId_, item.mediaType, item.totalCapacity, !isLoaded_);
        if (rc != EID(SUCCESS))
        {
            POS_TRACE_ERROR(EID(MFS_META_STORAGE_CREATE_FAILED),
                "Failed to create meta storage subsystem");
            return EID(MFS_META_STORAGE_CREATE_FAILED);
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
        if (!item.valid)
            continue;

        MetaVolumeType volumeType = MetaFileUtil::ConvertToVolumeType(item.mediaType);

        if (!ctrl->CreateVolume(volumeType))
        {
            POS_TRACE_ERROR(EID(MFS_META_VOLUME_CREATE_FAILED),
                "Error occurred to create volume (volume id={})",
                (int)volumeType);

            return EID(MFS_META_VOLUME_CREATE_FAILED);
        }
    }

    if (!mgmt->CreateMbr())
    {
        POS_TRACE_ERROR(EID(MFS_META_VOLUME_CREATE_FAILED),
            "Error occurred to create MetaFs MBR");

        return EID(MFS_META_VOLUME_CREATE_FAILED);
    }

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaFs::_OpenMetaVolume(void)
{
    POS_EVENT_ID rc = mgmt->LoadMbr(isNpor_);

    POSMetric metric(TEL40000_METAFS_NORMAL_SHUTDOWN, POSMetricTypes::MT_GAUGE);
    metric.AddLabel("array_id", to_string(arrayId_));
    metric.SetGaugeValue((int)isNpor_);
    telemetryPublisher_->PublishMetric(metric);

    if (rc != EID(SUCCESS))
    {
        if (mgmt->IsMbrClean())
        {
            POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
                "Mbr is clean. This array is mounted for the first time");

            if (false == _Initialize())
                return EID(MFS_MODULE_INIT_FAILED);

            rc = _CreateMetaVolume();

            if (rc != EID(SUCCESS))
            {
                return rc;
            }
        }
        else
        {
            return rc;
        }
    }

    if (!ctrl->OpenVolume(isNpor_))
    {
        return EID(MFS_META_VOLUME_OPEN_FAILED);
    }

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaFs::_CloseMetaVolume(void)
{
    bool resetCxt = false;
    if (!ctrl->CloseVolume(resetCxt))
    {
        // Reset MetaFS DRAM Context
        if (resetCxt == true)
            return EID(MFS_META_VOLUME_CLOSE_FAILED);
        else
            return EID(MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE);
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Meta filesystem has been unmounted...");

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaFs::_CreateRocksDBMetaFs(void)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    // TODO(sang7.park) : get rocksdb directory location from config file
    std::string metaRocksDir = MetaFsServiceSingleton::Instance()->GetConfigManager()->GetRocksDbPath();
    if (!std::experimental::filesystem::exists(metaRocksDir))
    {
        bool ret = std::experimental::filesystem::create_directory(metaRocksDir);
        if (ret)
        {
            MFS_TRACE_INFO(EID(ROCKSDB_MFS_DIR_CREATION_SUCCEED), "RocksDB metafs create directory : {}", metaRocksDir);
        }
        else
        {
            MFS_TRACE_WARN(EID(ROCKSDB_MFS_DIR_CREATION_FAILED), "RocksDB metafs create directory failed : {}", metaRocksDir);
        }
    }
    else
    {
        MFS_TRACE_INFO(EID(ROCKSDB_MFS_DIR_CREATION_SUCCEED), "RocksDB metafs omitted to create directory : {}, because it's already exists", metaRocksDir);
    }
    std::string pathName = metaRocksDir + "/" + arrayInfo_->GetName() + "_RocksMeta";
    rocksdb::Status status = rocksdb::DB::Open(options, pathName, &rocksMeta);
    fileDescriptorAllocator = new FileDescriptorAllocator();
    if (status.ok())
    {
        MFS_TRACE_INFO(EID(ROCKSDB_MFS_DB_OPEN_SUCCEED), "RocksDB Open succeed path : {}", pathName);
        return EID(SUCCESS);
    }
    else
    {
        MFS_TRACE_ERROR(EID(ROCKSDB_MFS_DB_OPEN_FAILED), "RocksDB Open failed path : {}", pathName);
        return EID(ROCKSDB_MFS_DB_OPEN_FAILED);
    }
}
void
MetaFs::_RegisterMediaInfoIfAvailable(PartitionType ptnType, MetaStorageInfoList& mediaList)
{
    std::shared_ptr<MetaStorageInfo> media = _MakeMetaStorageMediaInfo(ptnType);

    if (media->valid)
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "PartitionType {} is available, arrayId: {}",
            (int)ptnType, arrayId_);
    }
    else
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "PartitionType {} is not available, arrayId: {}",
            (int)ptnType, arrayId_);
    }

    mediaList.push_back(media);
}

std::shared_ptr<MetaStorageInfo>
MetaFs::_MakeMetaStorageMediaInfo(PartitionType ptnType)
{
    const PartitionLogicalSize* ptnSize = arrayInfo_->GetSizeInfo(ptnType);

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
        POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "ptnType: {}, totalStripes: {}", (int)ptnType, ptnSize->totalStripes);
        newInfo->valid = true;
        newInfo->mediaCapacity = static_cast<uint64_t>(ptnSize->totalStripes) *
            ptnSize->blksPerStripe * ArrayConfig::BLOCK_SIZE_BYTE;
    }

    return newInfo;
}
} // namespace pos
