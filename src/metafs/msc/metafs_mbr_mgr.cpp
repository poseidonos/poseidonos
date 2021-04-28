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

#include "src/metafs/msc/metafs_mbr_mgr.h"

#include <utility>

namespace pos
{
// MetaFsMBRManager metafsMBRMgr;

MetaFsMBRManager::MetaFsMBRManager(void)
: mbr(nullptr)
{
}
MetaFsMBRManager::~MetaFsMBRManager(void)
{
    if (nullptr != mbr)
    {
        delete mbr;
        mbr = nullptr;
    }
}

void
MetaFsMBRManager::Init(std::string arrayName, MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    assert(mediaType == MetaStorageType::SSD);
    this->arrayName = arrayName;

    // note that Meta filesystem's MBR is only available upon SSD meta volume
    SetRegionInfo(mediaType, baseLpn, maxLpn);

    if (nullptr == mbr)
    {
        mbr = new MetaFsMBR(MetaFsAnchorRegionType::MasterBootRecord, baseLpn);
    }

    mbr->ResetContent();
}

bool
MetaFsMBRManager::IsValidMBRExist(void)
{
    return mbr->IsValidMBRExist();
}

bool
MetaFsMBRManager::LoadMBR(void)
{
    assert(mbr != nullptr);

    if (false == mbr->Load())
    {
        return false;
    }

    return true;
}

void
MetaFsMBRManager::Bringup(void)
{
    assert(mbr != nullptr);
}

uint64_t
MetaFsMBRManager::GetEpochSignature(void)
{
    return mbr->GetEpochSignature();
}

bool
MetaFsMBRManager::SaveContent(void)
{
    assert(mbr != nullptr);

    if (true != mbr->Store())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Store I/O for MFS MBR content has failed...");

        return false;
    }

    return true;
}

MetaLpnType
MetaFsMBRManager::GetRegionSizeInLpn(void)
{
    return mbr->GetLpnCntOfRegion();
}

void
MetaFsMBRManager::Finalize(void)
{
}

void
MetaFsMBRManager::SetMss(MetaStorageSubsystem* mss)
{
    mbr->SetMss(mss);
}

bool
MetaFsMBRManager::CreateMBR(void)
{
    assert(mbr != nullptr);
    mbr->CreateMBR();

    return SaveContent();
}

void
MetaFsMBRManager::RegisterVolumeGeometry(MetaStorageInfo& mediaInfo)
{
    assert(mediaInfo.media < MetaStorageType::Max);

    MetaFsMBRContent* content = mbr->GetContent();
    MetaFsStorageIoInfo info;
    info.mediaType = mediaInfo.media;
    info.totalCapacity = mediaInfo.mediaCapacity;
    info.valid = true;
    memcpy(&content->geometry.mediaPartitionInfo[content->geometry.volumeInfo.totalFilesystemVolumeCnt], &info, sizeof(MetaFsStorageIoInfo));
    content->geometry.volumeInfo.totalFilesystemVolumeCnt++;
    assert(content->geometry.volumeInfo.totalFilesystemVolumeCnt <= MetaFsGeometryInfo::MAX_INFO_COUNT);

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Volume Geometry information registered. <mediaType={}, total capacity={} MB>",
        (int)info.mediaType,
        (info.totalCapacity / 1024 / 1024));
}

MetaFsStorageIoInfoList&
MetaFsMBRManager::GetAllStoragePartitionInfo(void)
{
    MetaFsMBRContent* content = mbr->GetContent();
    return content->geometry.mediaPartitionInfo;
}

void
MetaFsMBRManager::SetPowerStatus(bool isShutDownOff)
{
    mbr->SetPORStatus(isShutDownOff);
}

bool
MetaFsMBRManager::GetPowerStatus(void)
{
    return mbr->GetPORStatus();
}

void
MetaFsMBRManager::InvalidMBR(void)
{
    if (nullptr != mbr)
    {
        mbr->InvalidMBRSignature();

        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "The signature of MFS MBR on DRAM is corrupted during shutdown");
    }
}
} // namespace pos
