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

#include "mfs_mbr_mgr.h"

MetaFsMBRMgrClass mfsMBRMgr;

MetaFsMBRMgrClass::MetaFsMBRMgrClass(void)
: mbr(nullptr)
{
}
MetaFsMBRMgrClass::~MetaFsMBRMgrClass(void)
{
    if (nullptr != mbr)
    {
        delete mbr;
        mbr = nullptr;
    }
}

void
MetaFsMBRMgrClass::Init(MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn)
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
MetaFsMBRMgrClass::IsValidMBRExist(void)
{
    return mbr->IsValidMBRExist();
}

bool
MetaFsMBRMgrClass::LoadMBR(void)
{
    assert(mbr != nullptr);

    if (false == mbr->Load())
    {
        return false;
    }
    return true;
}

void
MetaFsMBRMgrClass::BuildMBR(void)
{
    assert(mbr != nullptr);
    mbr->BuildMBR();
}

void
MetaFsMBRMgrClass::Bringup(void)
{
    assert(mbr != nullptr);
    mbr->BuildMBR();
}

uint64_t
MetaFsMBRMgrClass::GetEpochSignature(void)
{
    return mbr->GetEpochSignature();
}

bool
MetaFsMBRMgrClass::SaveContent(void)
{
    assert(mbr != nullptr);

    if (true != mbr->Store())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_SAVE_FAILED,
            "Store I/O for MFS MBR content has failed...");

        return false;
    }

    return true;
}

MetaLpnType
MetaFsMBRMgrClass::GetRegionSizeInLpn(void)
{
    return mbr->GetLpnCntOfRegion();
}

void
MetaFsMBRMgrClass::Finalize(void)
{
}

bool
MetaFsMBRMgrClass::CreateMBR(void)
{
    assert(mbr != nullptr);
    mbr->CreateMBR();

    return SaveContent();
}

void
MetaFsMBRMgrClass::RegisterVolumeGeometry(MetaStorageMediaInfo& mediaInfo)
{
    assert(mediaInfo.media < MetaStorageType::Max);

    MetaFsMBRContent* content = mbr->GetContent();
    MetaFsStorageIoInfo info;
    info.mediaType = mediaInfo.media;
    info.totalCapacity = mediaInfo.mediaCapacity;
    info.valid = true;
    content->geometry.mediaPartitionInfo.push_back(info);
    content->geometry.volumeInfo.totalFilesystemVolumeCnt++;

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Volume Geometry information registered. <mediaType={}, total capacity={} MB>",
        (int)info.mediaType,
        (info.totalCapacity / 1024 / 1024));
}

MetaFsStorageIoInfo&
MetaFsMBRMgrClass::GetStoragePartitionInfo(MetaStorageType media)
{
    MetaFsMBRContent* content = mbr->GetContent();
    for (auto& item : content->geometry.mediaPartitionInfo)
    {
        if (item.mediaType == media)
        {
            return item;
        }
    }
    assert(false);
}

MetaFsStorageIoInfoList&
MetaFsMBRMgrClass::GetAllStoragePartitionInfo(void)
{
    MetaFsMBRContent* content = mbr->GetContent();
    return content->geometry.mediaPartitionInfo;
}

uint64_t
MetaFsMBRMgrClass::GetCapacity(MetaStorageType media)
{
    MetaFsMBRContent* content = mbr->GetContent();
    return content->geometry.mediaPartitionInfo[(int)media].totalCapacity;
}

void
MetaFsMBRMgrClass::SetPowerStatus(bool isShutDownOff)
{
    mbr->SetPORStatus(isShutDownOff);
}

bool
MetaFsMBRMgrClass::GetPowerStatus(void)
{
    return mbr->GetPORStatus();
}

void
MetaFsMBRMgrClass::InvalidMBR(void)
{
    if (nullptr != mbr)
    {
        mbr->InvalidMBRSignature();

        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "The signature of MFS MBR on DRAM is corrupted during shutdown");
    }
}
