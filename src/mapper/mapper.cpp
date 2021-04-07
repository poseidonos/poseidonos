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

#include "mapper.h"

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "map_flushed_event.h"
#include "reverse_map.h"
#include "src/allocator/allocator.h"
#include "src/allocator/stripe.h"
#include "src/array/array.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_manager.h"

namespace ibofos
{
Mapper::Mapper(void)
: vsaMapManager(nullptr),
  stripeMap(nullptr),
  revMapPacks(nullptr),
  revMapWholefile(nullptr)
{
    vsaMapManager = new VSAMapManager();
    pthread_rwlock_init(&stripeMapLock, nullptr);
}

Mapper::~Mapper(void)
{
    if (revMapPacks != nullptr)
    {
        delete[] revMapPacks;
        revMapPacks = nullptr;
    }

    if (revMapWholefile != nullptr)
    {
        revMapWholefile->Close();

        delete revMapWholefile;
        revMapWholefile = nullptr;
    }

    if (stripeMap != nullptr)
    {
        delete stripeMap;
        stripeMap = nullptr;
    }

    delete vsaMapManager;
    vsaMapManager = nullptr;
}

void
Mapper::Init(void)
{
    info.Load();
    _InitMetadata(info);
}

void
Mapper::_InitMetadata(const MapperAddressInfo& info)
{
    stripeMap = new StripeMapContent(STRIPE_MAP_ID);
    stripeMap->Prepare(info.maxVsid);
    stripeMap->LoadSync(CREATE_FILE_IF_NOT_EXIST);

    // Set special variables for ReverseMapPack::
    ReverseMapPack::SetPageSize();
    ReverseMapPack::SetNumMpages();

    // Create MFS and Open the file for whole reverse map
    revMapWholefile = new FILESTORE("RevMapWhole");
    if (revMapWholefile->DoesFileExist() == false)
    {
        uint64_t fileSize = ReverseMapPack::GetfileSizePerStripe() * info.maxVsid;
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::REVMAP_FILE_SIZE,
            "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
            ReverseMapPack::GetfileSizePerStripe(), info.maxVsid, fileSize);

        int ret = revMapWholefile->Create(fileSize);
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REVMAP_FILE_SIZE,
                "RevMapWhole file Create failed, ret:{}", ret);
            assert(false);
        }
    }
    revMapWholefile->Open();

    // Make ReverseMapPack:: objects by the number of WriteBuffer stripes
    revMapPacks = new ReverseMapPack[info.numWbStripes];
    for (StripeId wbLsid = 0; wbLsid < info.numWbStripes; ++wbLsid)
    {
        revMapPacks[wbLsid].Init(wbLsid);
    }
}

int
Mapper::SyncStore(void)
{
    int ret = 0;

    ret = stripeMap->StoreSync();
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "StripeMap Store failed");
        return ret;
    }

    ret = vsaMapManager->SyncStore();
    if (ret < 0)
    {
        return ret;
    }

    return ret;
}

int
Mapper::AsyncStore(void)
{
    int ret = 0;
    ret = vsaMapManager->AsyncStore();
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE),
            "AsyncStore() of vsaMapManager Failed, Check logs");
    }

    EventSmartPtr callBackStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, vsaMapManager);
    ret = stripeMap->Flush(callBackStripeMap);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "Flush() for stripeMap Failed");
    }

    return ret;
}

void
Mapper::Close(void)
{
    vsaMapManager->Close();
}

void
Mapper::RegisterToPublisher(void)
{
    vsaMapManager->RegisterToPublisher();
}

void
Mapper::RemoveFromPublisher(void)
{
    vsaMapManager->RemoveFromPublisher();
}

VirtualBlkAddr
Mapper::GetVSA(int volumeId, BlkAddr rba)
{
    VsaArray vsaArray;
    GetVSAs(volumeId, rba, 1 /* numBlks */, vsaArray);
    return vsaArray[0];
}

VirtualBlkAddr
Mapper::GetVSAInternal(int volumeId, BlkAddr rba, int& caller)
{
    int ret = vsaMapManager->EnableInternalAccess(volumeId, caller);
    if (CALLER_EVENT == caller)
    {
        if (vsaMapManager->IsVSAMapLoaded(volumeId) == false)
        {
            // [Exist Volume Case]
            // 0: The First Internal-Approach
            // -EID(MAP_LOAD_ONGOING): Subsequent Internal-Approaches
            if (0 == ret || -EID(MAP_LOAD_ONGOING) == ret)
            {
                caller = NEED_RETRY;
            }
            // [Deleted Volume Case]
            else if (-EID(VSAMAP_LOAD_FAILURE) == ret)
            {
                // Do nothing
            }
            return UNMAP_VSA;
        }
    }
    else if (ret < 0)
    {
        return UNMAP_VSA;
    }

    caller = OK_READY;
    return _ReadVSA(volumeId, rba);
}

VirtualBlkAddr
Mapper::_ReadVSA(int volumeId, BlkAddr rba)
{
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volumeId);
    return vsaMap->GetEntry(rba);
}

int
Mapper::GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks,
    VsaArray& vsaArray)
{
    if (false == vsaMapManager->IsVsaMapAccessible(volumeId))
    {
        IBOF_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE),
            "VolumeId:{} is not accessible, maybe unmounted", volumeId);
        for (uint32_t blkIdx = 0; blkIdx < numBlks; ++blkIdx)
        {
            vsaArray[blkIdx] = UNMAP_VSA;
        }
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }

    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volumeId);
    for (uint32_t blkIdx = 0; blkIdx < numBlks; ++blkIdx)
    {
        BlkAddr targetRba = startRba + blkIdx;
        vsaArray[blkIdx] = vsaMap->GetEntry(targetRba);
    }
    return 0;
}

int
Mapper::SetVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    if (false == vsaMapManager->IsVsaMapAccessible(volumeId))
    {
        IBOF_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE),
            "VolumeId:{} is not accessible, maybe unmounted", volumeId);
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }
    return _UpdateVsaMap(volumeId, startRba, virtualBlks);
}

int
Mapper::SetVsaMapInternal(int volumeId, BlkAddr startRba,
    VirtualBlks& virtualBlks)
{
    int caller = CALLER_NOT_EVENT;
    int ret = vsaMapManager->EnableInternalAccess(volumeId, caller);
    if (ret < 0)
    {
        return ret;
    }
    return _UpdateVsaMap(volumeId, startRba, virtualBlks);
}

int
Mapper::_UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int ret = 0;
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volumeId);

    for (uint32_t blkIdx = 0; blkIdx < virtualBlks.numBlks; blkIdx++)
    {
        VirtualBlkAddr targetVsa = {.stripeId = virtualBlks.startVsa.stripeId,
            .offset = virtualBlks.startVsa.offset + blkIdx};
        BlkAddr targetRba = startRba + blkIdx;
        ret = vsaMap->SetEntry(targetRba, targetVsa);
        if (ret < 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::VSAMAP_SET_FAILURE,
                "VSAMap set failure, volumeId:{}  targetRba:{}  targetVsa.sid:{}  targetVsa.offset:{}",
                volumeId, targetRba, targetVsa.stripeId, targetVsa.offset);
            break;
        }
    }
    return ret;
}

MpageList
Mapper::GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks)
{
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volumeId);
    return vsaMap->GetDirtyPages(startRba, numBlks);
}

int
Mapper::LinkReverseMap(Stripe* stripe, StripeId wbLsid, StripeId vsid)
{
    int ret = 0;

    // Let Stripe:: know the wbLsid(th) ReverseMapPack:: object
    ret = stripe->LinkReverseMap(&revMapPacks[wbLsid]);
    if (ret < 0)
    {
        return ret;
    }

    // Let wbLsid(th) ReverseMapPack:: object point vsid(SSD LSID)
    ReverseMapPack& revMapPack = revMapPacks[wbLsid];
    ret = revMapPack.LinkVsid(vsid);
    if (ret < 0)
    {
        return ret;
    }

    return ret;
}

int
Mapper::ResetVSARange(int volumeId, BlkAddr rba, uint64_t cnt)
{
    if (false == vsaMapManager->IsVsaMapAccessible(volumeId))
    {
        IBOF_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE),
            "VolumeId:{} is not accessible, maybe unmounted", volumeId);
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }

    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volumeId);
    vsaMap->ResetEntries(rba, cnt);

    return 0;
}

VirtualBlkAddr
Mapper::GetRandomVSA(BlkAddr rba)
{
    VirtualBlkAddr vsa;
    vsa.stripeId = rba / info.blksPerStripe;
    vsa.offset = rba % info.blksPerStripe;
    return vsa;
}

StripeId
Mapper::GetRandomLsid(StripeId vsid)
{
    return vsid + info.numWbStripes;
}

StripeAddr
Mapper::GetLSA(StripeId vsid)
{
    pthread_rwlock_rdlock(&stripeMapLock);
    StripeAddr stripeAddr = stripeMap->GetEntry(vsid);
    pthread_rwlock_unlock(&stripeMapLock);
    return stripeAddr;
}

int
Mapper::UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    StripeAddr entry = {.stripeLoc = loc, .stripeId = lsid};
    pthread_rwlock_wrlock(&stripeMapLock);
    int ret = stripeMap->SetEntry(vsid, entry);
    pthread_rwlock_unlock(&stripeMapLock);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::STRIPEMAP_SET_FAILURE,
            "StripeMap set failure, vsid:{}  lsid:{}  loc:{}", vsid, lsid, loc);
    }
    else
    {
        ret = 0;
    }
    return ret;
}

MpageList
Mapper::GetDirtyStripeMapPages(int vsid)
{
    return stripeMap->GetDirtyPages(vsid, 1);
}

int
Mapper::StartDirtyPageFlush(int mapId, MpageList dirtyPages,
    EventSmartPtr callback)
{
    MapContent* map = _GetMapContent(mapId);
    if (nullptr == map)
    {
        return -EID(WRONG_MAP_ID);
    }

    return map->StartDirtyPageFlush(dirtyPages, callback);
}

int
Mapper::FlushMap(int mapId, EventSmartPtr callback)
{
    MapContent* map = _GetMapContent(mapId);
    if (nullptr == map)
    {
        return -EID(WRONG_MAP_ID);
    }

    return map->Flush(callback);
}

MapContent*
Mapper::_GetMapContent(int mapId)
{
    MapContent* map = nullptr;

    if (mapId == STRIPE_MAP_ID)
    {
        map = stripeMap;
    }
    else if (0 <= mapId && mapId < MAX_VOLUME_COUNT)
    {
        map = vsaMapManager->GetVSAMapContent(mapId);
    }

    return map;
}

std::tuple<StripeAddr, bool>
Mapper::GetAndReferLsid(StripeId vsid)
{
    pthread_rwlock_rdlock(&stripeMapLock);
    StripeAddr stripeAddr = stripeMap->GetEntry(vsid);
    bool referenced = ReferLsid(stripeAddr);
    pthread_rwlock_unlock(&stripeMapLock);

    return std::make_tuple(stripeAddr, referenced);
}

bool
Mapper::ReferLsid(StripeAddr& lsidEntry)
{
    if (IsInUserDataArea(lsidEntry))
    {
        return false;
    }
    Stripe& stripe =
        *AllocatorSingleton::Instance()->GetStripe(lsidEntry);
    stripe.Refer();

    return true;
}

void
Mapper::DereferLsid(StripeAddr& lsidEntry, uint32_t blockCount)
{
    if (IsInUserDataArea(lsidEntry))
    {
        return;
    }
    Stripe& stripe =
        *AllocatorSingleton::Instance()->GetStripe(lsidEntry);
    stripe.Derefer(blockCount);
}

/*
 * File("fname") Format
 *
 *   Size         Title            Offset(mpageSize aligned)
 *   ---------------------------   0
 *   8B           numValidMpages
 *   8B           numTotalMpages
 *   8B * BitMap  numEntry
 *   ---------------------------   N (The header occupies N mpages)
 *   mpageSize    mpageNum 0
 *   ---------------------------   N + 1
 *   mpageSize    mpageNum 1
 *   ---------------------------   N + 2
 *   mpageSize    mpageNum 2
 *   ---------------------------   N + 3
 *   ...
 *   ---------------------------   N + numValidMpages
 */
int
Mapper::ReadVsaMap(int volId, std::string fname)
{
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volId);
    return vsaMap->Dump(fname);
}

int
Mapper::WriteVsaMap(int volId, std::string fname)
{
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volId);
    return vsaMap->DumpLoad(fname);
}

int
Mapper::ReadStripeMap(std::string fname)
{
    return stripeMap->Dump(fname);
}

int
Mapper::WriteStripeMap(std::string fname)
{
    return stripeMap->DumpLoad(fname);
}

int
Mapper::ReadVsaMapEntry(int volId, BlkAddr rba, std::string fname)
{
    VirtualBlkAddr vsa = GetVSA(volId, rba);

    ofstream out(fname.c_str(), std::ofstream::app);

    out << "rba: 0x" << std::hex << rba << std::endl;
    out << "vsid: 0x" << std::hex << vsa.stripeId << std::endl;
    out << "offset: 0x" << std::hex << vsa.offset << std::endl;
    out << std::endl;

    return 0;
}

int
Mapper::WriteVsaMapEntry(int volId, BlkAddr rba, VirtualBlkAddr vsa)
{
    VirtualBlks blks = {.startVsa = vsa, .numBlks = 1};
    int ret = _UpdateVsaMap(volId, rba, blks);
    return ret;
}

int
Mapper::ReadStripeMapEntry(StripeId vsid, std::string fname)
{
    StripeAddr entry = GetLSA(vsid);

    ofstream out(fname.c_str(), std::ofstream::app);

    out << "vsid: 0x" << std::hex << vsid << std::endl;
    out << "location: " << std::hex << entry.stripeLoc << std::endl;
    out << "lsid: 0x" << std::hex << entry.stripeId << std::endl;
    out << std::endl;

    return 0;
}

int
Mapper::WriteStripeMapEntry(StripeId vsid, StripeLoc loc, StripeId lsid)
{
    int ret = UpdateStripeMap(vsid, lsid, loc);
    return ret;
}

int
Mapper::ReadReverseMap(StripeId vsid, std::string fname)
{
    ReverseMapPack* reverseMapPack = new ReverseMapPack;

    int ret = _LoadReverseMapVsidFromMFS(reverseMapPack, vsid);
    if (ret < 0)
    {
        delete reverseMapPack;
        return ret;
    }

    // Store ReverseMap of vsid to Linux file(fname)
    MetaFileIntf* fileToStore = new MockFileIntf(fname);
    fileToStore->Create(reverseMapPack->GetfileSizePerStripe());
    fileToStore->Open();

    ret = reverseMapPack->WbtFileSyncIo(fileToStore, MetaFsIoOpcode::Write);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "WbtFileIo() Write failed");
    }

    fileToStore->Close();
    delete fileToStore;
    delete reverseMapPack;
    return ret;
}

int
Mapper::WriteReverseMap(StripeId vsid, std::string fname)
{
    ReverseMapPack* reverseMapPack = new ReverseMapPack;
    int ret = reverseMapPack->LinkVsid(vsid);
    if (ret < 0)
    {
        delete reverseMapPack;
        return ret;
    }

    // Load ReverseMap from Linux file(fname)
    MetaFileIntf* fileFromLoad = new MockFileIntf(fname);
    fileFromLoad->Open();

    ret = reverseMapPack->WbtFileSyncIo(fileFromLoad, MetaFsIoOpcode::Read);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "WbtFileIo() Read failed");
    }

    fileFromLoad->Close();
    delete fileFromLoad;

    ret = _StoreReverseMapToMFS(reverseMapPack);

    delete reverseMapPack;
    return ret;
}

int
Mapper::ReadWholeReverseMap(std::string fname)
{
    int ret = 0;
    uint64_t fileSize = ReverseMapPack::GetfileSizePerStripe() * info.maxVsid;
    char* buffer = new char[fileSize]();

    // Load Whole ReverseMap from MFS
    ret = revMapWholefile->IssueIO(MetaFsIoOpcode::Read, 0, fileSize, buffer);
    if (ret < 0)
    {
        delete[] buffer;
        return ret;
    }

    // Store Whole ReverseMap to Linux file(fname)
    MetaFileIntf* fileToStore = new MockFileIntf(fname);
    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::REVMAP_FILE_SIZE,
        "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
        ReverseMapPack::GetfileSizePerStripe(), info.maxVsid, fileSize);
    fileToStore->Create(fileSize);
    fileToStore->Open();

    fileToStore->IssueIO(MetaFsIoOpcode::Write, 0, fileSize, buffer);
    fileToStore->Close();

    delete fileToStore;
    delete[] buffer;
    return ret;
}

int
Mapper::WriteWholeReverseMap(std::string fname)
{
    int ret = 0;
    uint64_t fileSize = ReverseMapPack::GetfileSizePerStripe() * info.maxVsid;
    char* buffer = new char[fileSize]();

    // Load Whole ReverseMap from Linux file(fname)
    MetaFileIntf* filefromLoad = new MockFileIntf(fname);
    filefromLoad->Open();
    filefromLoad->IssueIO(MetaFsIoOpcode::Read, 0, fileSize, buffer);
    filefromLoad->Close();
    delete filefromLoad;

    // Store Whole ReverseMap to MFS
    ret = revMapWholefile->IssueIO(MetaFsIoOpcode::Write, 0, fileSize, buffer);

    delete[] buffer;
    return ret;
}

int
Mapper::ReadReverseMapEntry(StripeId vsid, BlkOffset offset, std::string fname)
{
    ReverseMapPack* reverseMapPack = new ReverseMapPack;

    int ret = _LoadReverseMapVsidFromMFS(reverseMapPack, vsid);

    // Print out to fname
    BlkAddr rba;
    uint32_t volId;
    std::tie(rba, volId) = reverseMapPack->GetReverseMapEntry(offset);

    std::ofstream outFile(fname, std::ofstream::app);

    outFile << "vsid: 0x" << std::hex << vsid << std::endl;
    outFile << "offset:0x" << std::hex << offset << std::endl;
    outFile << "rba: 0x" << std::hex << rba << std::endl;
    outFile << "volumeId: 0x" << std::hex << volId << std::endl;
    outFile << std::endl;

    delete reverseMapPack;
    return ret;
}

int
Mapper::WriteReverseMapEntry(StripeId vsid, BlkOffset offset, BlkAddr rba,
    uint32_t volumeId)
{
    ReverseMapPack* reverseMapPack = new ReverseMapPack;

    int ret = _LoadReverseMapVsidFromMFS(reverseMapPack, vsid);
    if (ret < 0)
    {
        delete reverseMapPack;
        return ret;
    }

    // Update entry
    reverseMapPack->SetReverseMapEntry(offset, rba, volumeId);

    ret = _StoreReverseMapToMFS(reverseMapPack);

    delete reverseMapPack;
    return ret;
}

int
Mapper::_LoadReverseMapVsidFromMFS(ReverseMapPack* reverseMapPack, StripeId vsid)
{
    int ret = reverseMapPack->LinkVsid(vsid);
    if (ret < 0)
    {
        return ret;
    }

    ret = reverseMapPack->Load(nullptr);
    if (ret < 0)
    {
        return ret;
    }

    int pagesToWait = 0;
    while (0 != (pagesToWait = reverseMapPack->IsAsyncIoDone()))
    {
        usleep(1);
    }

    return ret;
}

int
Mapper::_StoreReverseMapToMFS(ReverseMapPack* reverseMapPack)
{
    int ret = reverseMapPack->Flush(nullptr, nullptr);
    if (ret < 0)
    {
        return ret;
    }

    int pagesToWait = 0;
    while (0 != (pagesToWait = reverseMapPack->IsAsyncIoDone()))
    {
        usleep(1);
    }

    return ret;
}

int
Mapper::GetMapLayout(std::string fname)
{
    ofstream out(fname.c_str(), std::ofstream::app);

    out << "Stripe Location: " << IN_WRITE_BUFFER_AREA << "(WRITE_BUFFER), "
        << IN_USER_AREA << "(USER_AREA) " << std::endl;

    out << "VSA map entry size: 0x" << std::hex
        << sizeof(VirtualBlkAddr) << std::endl;
    out << "Stripe map entry size: 0x" << std::hex
        << sizeof(StripeAddr) << std::endl;

    out << "VSA stripe id bit length: " << std::dec << STRIPE_ID_BIT_LEN << std::endl;
    out << "VSA block offset bit length: " << BLOCK_OFFSET_BIT_LEN << std::endl;

    out << "Stripe map location bit length: " << STRIPE_LOC_BIT_LEN << std::endl;
    out << "Stripe map stripe id bit length: " << STRIPE_ID_BIT_LEN << std::endl;

    if (stripeMap == nullptr)
    {
        out << "Please create array and mount ibofos to see stripe map mpage info" << std::endl;
    }
    else
    {
        out << "Meta page size: 0x" << std::hex
            << stripeMap->GetPageSize() << std::endl;
        out << "Stripe map entries per mpage: 0x" << std::hex
            << stripeMap->GetEntriesPerPage() << std::endl;

        VSAMapContent* validVsaMap = _GetFirstValidVolume();
        if (validVsaMap == nullptr)
        {
            out << "Please create volume to see volume map mpage info" << std::endl;
        }
        else
        {
            out << "VSA map entries per mpage: 0x" << std::hex
                << validVsaMap->GetEntriesPerPage() << std::endl;
        }
    }

    out << std::endl;
    out.close();
    return 0;
}

VSAMapContent*
Mapper::_GetFirstValidVolume(void)
{
    VSAMapContent* vsaMap = nullptr;

    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        vsaMap = vsaMapManager->GetVSAMapContent(volumeId);
        if (vsaMap != nullptr)
        {
            return vsaMap;
        }
    }
    return nullptr;
}

int64_t
Mapper::GetNumUsedBlocks(int volId)
{
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volId);
    if (vsaMap == nullptr)
    {
        return -(int64_t)IBOF_EVENT_ID::VSAMAP_NULL_PTR;
    }

    return vsaMap->GetNumUsedBlocks();
}

void
Mapper::CheckMapStoreDone(void)
{
    while (vsaMapManager->AllMapsAsyncFlushed() == false)
    {
        usleep(1);
    }
    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "All VSAMaps and StripeMap are AsyncStored");
}

MetaFileIntf*
Mapper::GetRevMapWholeFile(void)
{
    return revMapWholefile;
}

} // namespace ibofos
