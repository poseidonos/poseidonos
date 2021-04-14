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

#include "metafs_mbr_mgr.h"

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
MetaFsMBRManager::LoadMBR(std::string arrayName)
{
    assert(mbr != nullptr);
    this->arrayName = arrayName;

    if (false == mbr->Load(arrayName))
    {
        return false;
    }
    return true;
}

void
MetaFsMBRManager::BuildMBR(void)
{
    assert(mbr != nullptr);
    mbr->BuildMBR();
}

void
MetaFsMBRManager::Bringup(void)
{
    assert(mbr != nullptr);
    mbr->BuildMBR();
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

    if (true != mbr->Store(arrayName))
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

bool
MetaFsMBRManager::CreateMBR(std::string arrayName)
{
    assert(mbr != nullptr);
    mbr->CreateMBR();
    this->arrayName = arrayName;

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

MetaVolumeMbrMap::MetaVolumeMbrMap(void)
{
    mbrBitmap = new BitMap(MetaFsConfig::MAX_ARRAY_CNT);
    mbrBitmap->ResetBitmap();
    mbrMap.clear();

    for (uint32_t index = 0; index < MetaFsConfig::MAX_ARRAY_CNT; index++)
    {
        mbrList[index] = nullptr;
    }
}

MetaVolumeMbrMap::~MetaVolumeMbrMap(void)
{
    delete mbrBitmap;
    mbrMap.clear();

    for (uint32_t index = 0; index < MetaFsConfig::MAX_ARRAY_CNT; index++)
    {
        if (nullptr != mbrList[index])
        {
            delete mbrList[index];
        }
    }
}

void
MetaVolumeMbrMap::Init(std::string& arrayName, MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    uint32_t index = mbrBitmap->FindFirstZero();
    mbrBitmap->SetBit(index);
    mbrMap.insert(std::pair<std::string, uint32_t>(arrayName, index));
    mbrList[index] = new MetaFsMBRManager();
    mbrList[index]->Init(arrayName, mediaType, baseLpn, maxLpn);
}

void
MetaVolumeMbrMap::Remove(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    if (mbr == mbrMap.end())
        return;

    uint32_t index = mbr->second;
    mbrBitmap->ClearBit(index);
    mbrMap.erase(mbr);
    delete mbrList[index];
    mbrList[index] = nullptr;
}

void
MetaVolumeMbrMap::RegisterVolumeGeometry(std::string& arrayName, MetaStorageInfo& mediaInfo)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    mbrList[mbr->second]->RegisterVolumeGeometry(mediaInfo);
}

bool
MetaVolumeMbrMap::IsValidMBRExist(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->IsValidMBRExist();
}

MetaFsStorageIoInfoList&
MetaVolumeMbrMap::GetAllStoragePartitionInfo(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->GetAllStoragePartitionInfo();
}

MetaLpnType
MetaVolumeMbrMap::GetRegionSizeInLpn(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->GetRegionSizeInLpn();
}

bool
MetaVolumeMbrMap::CreateMBR(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->CreateMBR(arrayName);
}

bool
MetaVolumeMbrMap::LoadMBR(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->LoadMBR(arrayName);
}

void
MetaVolumeMbrMap::InvalidMBR(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    mbrList[mbr->second]->InvalidMBR();
}

uint64_t
MetaVolumeMbrMap::GetEpochSignature(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);

    // can be first read
    if (mbr == mbrMap.end())
        return 0;

    return mbrList[mbr->second]->GetEpochSignature();
}

void
MetaVolumeMbrMap::SetPowerStatus(std::string& arrayName, bool isNPOR)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    mbrList[mbr->second]->SetPowerStatus(isNPOR);
}

bool
MetaVolumeMbrMap::GetPowerStatus(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->GetPowerStatus();
}

bool
MetaVolumeMbrMap::SaveContent(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    assert(mbr != mbrMap.end());

    return mbrList[mbr->second]->SaveContent();
}

bool
MetaVolumeMbrMap::IsMbrLoaded(std::string& arrayName)
{
    auto mbr = mbrMap.find(arrayName);
    if (mbr != mbrMap.end())
        return true;
    else
        return false;
}

uint32_t
MetaVolumeMbrMap::GetMountedMbrCount(void)
{
    return mbrMap.size();
}
} // namespace pos
