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

#include "src/metafs/include/metafs_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/mapper/reversemap/reversemap_manager.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/include/meta_const.h"
#include "src/metafs/metafs_file_intf.h"

#include <string>

namespace pos
{
ReverseMapManager::ReverseMapManager(VSAMapManager* ivsaMap, IStripeMap* istripeMap, IVolumeManager* vol, MapperAddressInfo* addrInfo_)
: mpageSize(0),
  numMpagesPerStripe(0),
  fileSizePerStripe(0),
  fileSizeWholeRevermap(0),
  revMapPacks(nullptr),
  revMapWholefile(nullptr),
  iVSAMap(ivsaMap),
  iStripeMap(istripeMap),
  volumeManager(vol),
  addrInfo(addrInfo_)
{
}
// LCOV_EXCL_START
ReverseMapManager::~ReverseMapManager(void)
{
    if (revMapWholefile != nullptr)
    {
        if (revMapWholefile->IsOpened() == true)
        {
            revMapWholefile->Close();
        }
        delete revMapWholefile;
        revMapWholefile = nullptr;
    }
    if (revMapPacks != nullptr)
    {
        delete [] revMapPacks;
        revMapPacks = nullptr;
    }
}
// LCOV_EXCL_STOP
void
ReverseMapManager::Init(void)
{
    if (volumeManager == nullptr)
    {
        volumeManager = VolumeServiceSingleton::Instance()->GetVolumeManager(addrInfo->GetArrayName());
    }

    _SetNumMpages();

    // Create MFS and Open the file for whole reverse map
    if (addrInfo->IsUT() == false)
    {
        revMapWholefile = new MetaFsFileIntf("RevMapWhole", addrInfo->GetArrayId());
    }
    else
    {
        revMapWholefile = new MockFileIntf("RevMapWhole", addrInfo->GetArrayId());
    }
    if (revMapWholefile->DoesFileExist() == false)
    {
        POS_TRACE_INFO(EID(REVMAP_FILE_SIZE), "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
                        fileSizePerStripe, addrInfo->GetMaxVSID(), fileSizeWholeRevermap);

        int ret = revMapWholefile->Create(fileSizeWholeRevermap);
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(REVMAP_FILE_SIZE), "RevMapWhole file Create failed, ret:{}", ret);
            assert(false);
        }
    }
    revMapWholefile->Open();

    // Make ReverseMapPack:: objects by the number of WriteBuffer stripes
    uint32_t numWbStripes = addrInfo->GetNumWbStripes();
    revMapPacks = new ReverseMapPack[numWbStripes]();
    for (StripeId wbLsid = 0; wbLsid < numWbStripes; ++wbLsid)
    {
        revMapPacks[wbLsid].Init(revMapWholefile, wbLsid, UINT32_MAX, mpageSize, numMpagesPerStripe);
    }
}

void
ReverseMapManager::Dispose(void)
{
    if (revMapWholefile != nullptr)
    {
        if (revMapWholefile->IsOpened() == true)
        {
            revMapWholefile->Close();
        }
        delete revMapWholefile;
        revMapWholefile = nullptr;
    }
    if (revMapPacks != nullptr)
    {
        delete [] revMapPacks;
        revMapPacks = nullptr;
    }
}

int
ReverseMapManager::Load(ReverseMapPack* rev, StripeId wblsid, StripeId vsid, EventSmartPtr cb)
{
    uint64_t fileOffset = fileSizePerStripe * vsid;
    if (rev != nullptr)
    {
        return rev->Load(fileOffset, cb, vsid);
    }
    else
    {
        return revMapPacks[wblsid].Load(fileOffset, cb, vsid);
    }
}

int
ReverseMapManager::Flush(ReverseMapPack* rev, StripeId wblsid, Stripe* stripe, StripeId vsid, EventSmartPtr cb)
{
    uint64_t fileOffset = fileSizePerStripe * vsid;
    if (rev != nullptr)
    {
        return rev->Flush(stripe, fileOffset, cb, vsid);
    }
    else
    {
        return revMapPacks[wblsid].Flush(stripe, fileOffset, cb, vsid);
    }
}

int
ReverseMapManager::UpdateReverseMapEntry(ReverseMapPack* rev, StripeId wblsid, uint64_t offset, BlkAddr rba, uint32_t volumeId)
{
    if (rev != nullptr)
    {
        return rev->SetReverseMapEntry(offset, rba, volumeId);
    }
    else
    {
        return revMapPacks[wblsid].SetReverseMapEntry(offset, rba, volumeId);
    }
}

std::tuple<BlkAddr, uint32_t>
ReverseMapManager::GetReverseMapEntry(ReverseMapPack* rev, StripeId wblsid, uint64_t offset)
{
    if (rev != nullptr)
    {
        return rev->GetReverseMapEntry(offset);
    }
    else
    {
        return revMapPacks[wblsid].GetReverseMapEntry(offset);
    }
}

ReverseMapPack*
ReverseMapManager::Assign(StripeId wblsid, StripeId vsid)
{
    revMapPacks[wblsid].Assign(vsid);
    return &revMapPacks[wblsid];
}

ReverseMapPack*
ReverseMapManager::AllocReverseMapPack(uint32_t vsid)
{
    ReverseMapPack* revPack = new ReverseMapPack();
    revPack->Init(revMapWholefile, UNMAP_STRIPE, vsid, mpageSize, numMpagesPerStripe);
    revPack->Assign(vsid);
    return revPack;
}

uint64_t
ReverseMapManager::GetReverseMapPerStripeFileSize(void)
{
    return fileSizePerStripe;
}

uint64_t
ReverseMapManager::GetWholeReverseMapFileSize(void)
{
    return fileSizeWholeRevermap;
}

int
ReverseMapManager::ReconstructReverseMap(uint32_t volumeId, uint64_t totalRbaNum, uint32_t wblsid, uint32_t vsid, uint64_t blockCount, std::map<uint64_t, BlkAddr> revMapInfos)
{
    int ret = 0;
    BlkAddr lastFoundRba = UINT64_MAX;
    POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_FOUND_RBA), "[ReconstructMap] START, volumeId:{}  wbLsid:{}  vsid:{}  blockCount:{}", volumeId, wblsid, vsid, blockCount);

    for (uint64_t offset = 0; offset < blockCount; offset++)
    {
        if (revMapInfos.find(offset) == revMapInfos.end())
        {
            // Set the RBA to start finding
            BlkAddr rbaStart = 0;
            if (lastFoundRba != UINT64_MAX)
            {
                rbaStart = lastFoundRba + 1;
            }

            BlkAddr foundRba = INVALID_RBA;
            bool found = _FindRba(volumeId, totalRbaNum, vsid, wblsid, offset, rbaStart, foundRba);
            if (found == false)
            {
                continue;
            }
            revMapInfos.insert(make_pair(offset, foundRba));
            lastFoundRba = foundRba;
        }
        revMapPacks[wblsid].SetReverseMapEntry(offset, revMapInfos[offset], volumeId);
    }

    POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_FOUND_RBA), "[ReconstructMap] {}/{} blocks are reconstructed for Stripe(wbLsid:{})", revMapInfos.size(), blockCount, wblsid);
    return ret;
}

int
ReverseMapManager::LoadReverseMapForWBT(MetaFileIntf* fileLinux, uint64_t offset,  uint64_t fileSize, char* buf)
{
    if (fileLinux == nullptr)
    {
        fileLinux = revMapWholefile;
    }
    return fileLinux->IssueIO(MetaFsIoOpcode::Read, offset, fileSize, buf);
}

int
ReverseMapManager::StoreReverseMapForWBT(MetaFileIntf* fileLinux, uint64_t offset, uint64_t fileSize, char* buf)
{
    if (fileLinux == nullptr)
    {
        fileLinux = revMapWholefile;
    }
    return fileLinux->IssueIO(MetaFsIoOpcode::Write, offset, fileSize, buf);
}

char*
ReverseMapManager::GetReverseMapPtrForWBT(void)
{
    return revMapPacks[0].GetRevMapPtrForWBT();
}

bool
ReverseMapManager::_FindRba(uint32_t volumeId, uint64_t totalRbaNum, StripeId vsid, StripeId wblsid, uint64_t offset, BlkAddr rbaStart, BlkAddr& foundRba)
{
    assert(totalRbaNum > 0);

    bool looped = false;
    for (BlkAddr rbaToCheck = rbaStart; rbaToCheck <= totalRbaNum; ++rbaToCheck)
    {
        // If rbaToCheck exceeds more than totalRbaNum, looped would be set as TRUE
        if (rbaToCheck == totalRbaNum)
        {
            rbaToCheck = 0;
            looped = true;
        }

        if ((rbaToCheck == rbaStart) && looped)
        {
            return false;
        }

        VirtualBlkAddr vsaToCheck = iVSAMap->GetVSAWoCond(volumeId, rbaToCheck);
        if (vsaToCheck == UNMAP_VSA || vsaToCheck.offset != offset || vsaToCheck.stripeId != vsid)
        {
            continue;
        }

        StripeAddr lsaToCheck = iStripeMap->GetLSA(vsaToCheck.stripeId);
        if (iStripeMap->IsInUserDataArea(lsaToCheck) || lsaToCheck.stripeId != wblsid)
        {
            continue;
        }

        foundRba = rbaToCheck;
        return true;
    }

    return false;
}

void
ReverseMapManager::WaitAllPendingIoDone(void)
{
    uint32_t numWbStripes = addrInfo->GetNumWbStripes();
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper ReverseMapMAnager] Wait For Pending IO");
    for (StripeId wbLsid = 0; wbLsid < numWbStripes; ++wbLsid)
    {
        revMapPacks[wbLsid].WaitPendingIoDone();
    }
}

int
ReverseMapManager::_SetNumMpages(void)
{
    int maxVsid = addrInfo->GetMaxVSID();
    mpageSize = addrInfo->GetMpageSize();
    uint32_t blksPerStripe = addrInfo->GetBlksPerStripe();
    uint32_t entriesPerNormalPage = (mpageSize / REVMAP_SECTOR_SIZE) * (REVMAP_SECTOR_SIZE / REVMAP_ENTRY_SIZE);
    uint32_t entriesPerFirstPage = entriesPerNormalPage - (REVMAP_SECTOR_SIZE / REVMAP_ENTRY_SIZE);

    if (blksPerStripe <= entriesPerFirstPage)
    {
        numMpagesPerStripe = 1;
    }
    else
    {
        numMpagesPerStripe = 1 + DivideUp(blksPerStripe - entriesPerFirstPage, entriesPerNormalPage);
    }

    fileSizePerStripe = mpageSize * numMpagesPerStripe;
    fileSizeWholeRevermap = fileSizePerStripe * maxVsid;

    POS_TRACE_INFO(EID(REVMAP_FILE_SIZE), "[ReverseMap Info] entriesPerNormalPage:{}  entriesPerFirstPage:{}  numMpagesPerStripe:{}  fileSizePerStripe:{}",
        entriesPerNormalPage, entriesPerFirstPage, numMpagesPerStripe, fileSizePerStripe);

    return 0;
}

} // namespace pos
