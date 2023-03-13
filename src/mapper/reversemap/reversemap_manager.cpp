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

#include "src/mapper/reversemap/reversemap_manager.h"

#include <map>
#include <string>
#include <tuple>
#include <utility>

#include "src/array_mgmt/array_manager.h"
#include "src/include/meta_const.h"
#include "src/mapper/reversemap/reverse_map_io.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/meta_file_intf/rocksdb_metafs_intf.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/metafs_file_intf.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
ReverseMapManager::ReverseMapManager(IVSAMap* ivsaMap, IStripeMap* istripeMap, IVolumeInfoManager* vol, MapperAddressInfo* addrInfo_, TelemetryPublisher* tp)
: numMpagesPerStripe(0),
  fileSizePerStripe(0),
  fileSizeWholeRevermap(0),
  counts{},
  revMapWholefile(nullptr),
  iVSAMap(ivsaMap),
  iStripeMap(istripeMap),
  volumeManager(vol),
  addrInfo(addrInfo_),
  telemetryPublisher(tp),
  rocksDbEnabled(MetaFsServiceSingleton::Instance()->GetConfigManager()->IsRocksdbEnabled())
{
}
// LCOV_EXCL_START
ReverseMapManager::~ReverseMapManager(void)
{
    if (revMapWholefile != nullptr)
    {
        if (addrInfo->IsUT() == true)
        {
            // TODO Move this code to test code (and inject metafile dependency in constructor)
            revMapWholefile->Delete();
        }
        if (revMapWholefile->IsOpened() == true)
        {
            revMapWholefile->Close();
        }
        delete revMapWholefile;
        revMapWholefile = nullptr;
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
        if (rocksDbEnabled)
        {
            revMapWholefile = new RocksDBMetaFsIntf("RevMapWhole", addrInfo->GetArrayId(), MetaFileType::SpecialPurposeMap);
            POS_TRACE_INFO(EID(REVMAP_INITIALIZED),
                "RocksDBMetaFsIntf for reverse map has been initialized , fileName : {} , arrayId : {} ", "RevMapWhole", addrInfo->GetArrayId());
        }
        else
        {
            revMapWholefile = new MetaFsFileIntf("RevMapWhole", addrInfo->GetArrayId(), MetaFileType::SpecialPurposeMap);
            POS_TRACE_INFO(EID(REVMAP_INITIALIZED),
                "MetaFsFileIntffor reverse map has been initialized , fileName : {} , arrayId : {} ", "RevMapWhole", addrInfo->GetArrayId());
        }
    }
    else
    {
        revMapWholefile = new MockFileIntf("RevMapWhole", addrInfo->GetArrayId(), MetaFileType::SpecialPurposeMap);
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
}

uint64_t
ReverseMapManager::_GetFileOffset(StripeId vsid)
{
    return fileSizePerStripe * vsid;
}

int
ReverseMapManager::Load(ReverseMapPack* rev, EventSmartPtr cb)
{
    assert(rev != nullptr);
    ReverseMapIo* reverseMapLoadContext = _CreateIoContext(rev, cb, IoDirection::IO_LOAD);

    counts[IoDirection::IO_LOAD].issuedCount++;

    return reverseMapLoadContext->Load();
}

int
ReverseMapManager::Flush(ReverseMapPack* rev, EventSmartPtr cb)
{
    assert(rev != nullptr);
    ReverseMapIo* reverseMapFlushContext = _CreateIoContext(rev, cb, IoDirection::IO_FLUSH);

    counts[IoDirection::IO_FLUSH].issuedCount++;

    return reverseMapFlushContext->Flush();
}

ReverseMapIo*
ReverseMapManager::_CreateIoContext(ReverseMapPack* rev, EventSmartPtr cb, IoDirection direction)
{
    StripeId vsid = rev->GetVsid();

    return new ReverseMapIo(rev, cb,
        revMapWholefile, _GetFileOffset(vsid), direction, telemetryPublisher,
        EventSchedulerSingleton::Instance(),
        std::bind(&ReverseMapManager::_ReverseMapIoDone, this, std::placeholders::_1));
}

void
ReverseMapManager::_ReverseMapIoDone(ReverseMapIo* reverseMapIo)
{
    counts[reverseMapIo->GetIoDirection()].completedCount++;
    delete reverseMapIo;
}

ReverseMapPack*
ReverseMapManager::AllocReverseMapPack(StripeId vsid, StripeId wblsid)
{
    ReverseMapPack* revPack = new ReverseMapPack(vsid, wblsid,
        addrInfo->GetMpageSize(), numMpagesPerStripe);
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
ReverseMapManager::ReconstructReverseMap(uint32_t volumeId, uint64_t totalRbaNum, uint32_t wblsid, uint32_t vsid, uint64_t blockCount, std::map<uint64_t, BlkAddr> revMapInfos, ReverseMapPack* revMapPack)
{
    int ret = 0;
    POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_FOUND_RBA), "[ReconstructMap] START, volumeId:{}  wbLsid:{}  vsid:{}  blockCount:{}", volumeId, wblsid, vsid, blockCount);

    // construct the map of stripe id to offset to rba
    if (!invertedMap[volumeId].size())
    {
        _ConstructInvertedMap(volumeId, totalRbaNum);
    }

    // verify
    _CheckInvertedMapValid(volumeId, vsid, revMapInfos);

    // look up rba
    for (uint64_t offset = 0; offset < blockCount; offset++)
    {
        auto foundRba = _FindRba(volumeId, {vsid, offset});
        if (foundRba.first)
        {
            revMapPack->SetReverseMapEntry(offset, foundRba.second, volumeId);
        }
    }

    POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_FOUND_RBA), "[ReconstructMap] {}/{} blocks are reconstructed for Stripe(wbLsid:{})", revMapInfos.size(), blockCount, wblsid);
    return ret;
}

int
ReverseMapManager::LoadReverseMapForWBT(uint64_t offset, uint64_t fileSize, char* buf)
{
    return revMapWholefile->IssueIO(MetaFsIoOpcode::Read, offset, fileSize, buf);
}

int
ReverseMapManager::StoreReverseMapForWBT(uint64_t offset, uint64_t fileSize, char* buf)
{
    return revMapWholefile->IssueIO(MetaFsIoOpcode::Write, offset, fileSize, buf);
}

std::pair<bool, BlkAddr>
ReverseMapManager::_FindRba(const uint32_t volumeId, const VirtualBlkAddr addr)
{
    auto result = invertedMap[volumeId][addr.stripeId].find((uint64_t)addr.offset);
    if (result == invertedMap[volumeId][addr.stripeId].end())
    {
        return {false, -1};
    }
    return {true, result->second};
}

void
ReverseMapManager::_ConstructInvertedMap(const uint32_t volumeId, const uint64_t totalRbaNum)
{
    for (uint64_t rba = 0; rba < totalRbaNum; rba++)
    {
        auto vsaToCheck = iVSAMap->GetVSAWithSyncOpen(volumeId, rba);
        if (vsaToCheck == UNMAP_VSA)
        {
            continue;
        }
        invertedMap[volumeId][vsaToCheck.stripeId].insert({(uint64_t)vsaToCheck.offset, rba});
    }
}

void
ReverseMapManager::_CheckInvertedMapValid(const uint32_t volumeId, const uint32_t vsid, const std::map<uint64_t, BlkAddr> revMapInfos)
{
    auto iter = revMapInfos.begin();
    while (iter != revMapInfos.end())
    {
        auto rbaInLog = iter->second;
        auto blockOffsetInTheStripe = iter->first;
        auto result = invertedMap[volumeId][vsid].find(blockOffsetInTheStripe);
        if (result != invertedMap[volumeId][vsid].end() && result->second != rbaInLog)
        {
            POS_TRACE_ERROR(EID(REVMAP_RBA_MISMATCH_BETWEEN_LOG_AND_INVMAP),
                "RBA cannot be found, vsid:{}, result->second:{}, rbaInLog:{}",
                vsid, result->second, rbaInLog);
            assert(false);
        }
        ++iter;
    }
}

void
ReverseMapManager::WaitAllPendingIoDone(void)
{
    // TODO
}

int
ReverseMapManager::_SetNumMpages(void)
{
    int maxVsid = addrInfo->GetMaxVSID();
    uint64_t mpageSize = addrInfo->GetMpageSize();
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
