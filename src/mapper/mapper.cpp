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

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "src/mapper/mapper.h"
#include "src/mapper/map_flushed_event.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/mapper_service/mapper_service.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_manager.h"

namespace pos
{

MpageList IMapFlush::DEFAULT_DIRTYPAGE_SET;

Mapper::Mapper(IArrayInfo* iarrayInfo, IStateControl* iState)
: iArrayinfo(iarrayInfo),
  iStateControl(iState),
  isInitialized(false)
{
    addrInfo = new MapperAddressInfo();
    vsaMapManager = new VSAMapManager(addrInfo, iarrayInfo->GetName());
    stripeMapManager = new StripeMapManager(addrInfo, iarrayInfo->GetName());
    reverseMapManager = new ReverseMapManager(vsaMapManager->GetIVSAMap(), stripeMapManager, iArrayinfo);
}

Mapper::~Mapper(void)
{
    delete reverseMapManager;
    delete stripeMapManager;
    delete vsaMapManager;
    delete addrInfo;
}

int
Mapper::Init(void)
{
    if (isInitialized == false)
    {
        addrInfo->SetupAddressInfo(iArrayinfo->GetName());
        vsaMapManager->Init();
        stripeMapManager->Init(*addrInfo);
        reverseMapManager->Init(*addrInfo);
        _RegisterToMapperService();
        isInitialized = true;
    }

    return 0;
}

void
Mapper::Dispose(void)
{
    if (isInitialized == true)
    {
        FlushAllMaps();
        WaitForFlushAllMapsDone();
        Close();
        _UnregisterFromMapperService();
        isInitialized = false;
    }
}

void
Mapper::Close(void)
{
    vsaMapManager->Close();
    stripeMapManager->Close();
    reverseMapManager->Close();
}

IVSAMap*
Mapper::GetIVSAMap(void)
{
    return vsaMapManager->GetIVSAMap();
}

IStripeMap*
Mapper::GetIStripeMap(void)
{
    return stripeMapManager;
}

IReverseMap*
Mapper::GetIReverseMap(void)
{
    return reverseMapManager;
}

IMapFlush*
Mapper::GetIMapFlush(void)
{
    return this;
}

IMapperWbt*
Mapper::GetIMapperWbt(void)
{
    return this;
}

int
Mapper::FlushDirtyMpages(int mapId, EventSmartPtr callback, MpageList dirtyPages)
{
    MapContent* map = _GetMapContent(mapId);
    if (nullptr == map)
    {
        return -EID(WRONG_MAP_ID);
    }

    if (IMapFlush::DEFAULT_DIRTYPAGE_SET == dirtyPages)
    {
        return map->FlushTouchedPages(callback);
    }
    else
    {
        return map->FlushDirtyPagesGiven(dirtyPages, callback);
    }
}

int
Mapper::FlushAllMaps(void)
{
    int ret = 0;

    ret = vsaMapManager->FlushMaps();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE), "AsyncStore() of vsaMapManager Failed, Check logs");
    }

    ret = stripeMapManager->FlushMap();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "AsyncStore() of stripeMapManager Failed, Check logs");
    }

    return ret;
}

int
Mapper::StoreAllMaps(void)
{
    int ret = 0;

    ret = stripeMapManager->StoreMap();
    if (ret < 0)
    {
        return ret;
    }

    ret = vsaMapManager->StoreMaps();
    if (ret < 0)
    {
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------

MapContent*
Mapper::_GetMapContent(int mapId)
{
    MapContent* map = nullptr;

    if (mapId == STRIPE_MAP_ID)
    {
        map = stripeMapManager->GetStripeMapContent();
    }
    else if (0 <= mapId && mapId < MAX_VOLUME_COUNT)
    {
        map = vsaMapManager->GetVSAMapContent(mapId);
    }

    return map;
}

void
Mapper::_RegisterToMapperService(void)
{
    std::string arrayName = iArrayinfo->GetName();
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->RegisterMapper(arrayName, GetIVSAMap());
    mapperService->RegisterMapper(arrayName, GetIStripeMap());
    mapperService->RegisterMapper(arrayName, GetIReverseMap());
    mapperService->RegisterMapper(arrayName, GetIMapFlush());
    mapperService->RegisterMapper(arrayName, GetIMapperWbt());
}

void
Mapper::_UnregisterFromMapperService(void)
{
    std::string arrayName = iArrayinfo->GetName();
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->UnregisterMapper(arrayName);
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
    return vsaMap->Dump(fname, iArrayinfo->GetName());
}

int
Mapper::WriteVsaMap(int volId, std::string fname)
{
    VSAMapContent* vsaMap = vsaMapManager->GetVSAMapContent(volId);
    return vsaMap->DumpLoad(fname, iArrayinfo->GetName());
}

int
Mapper::ReadStripeMap(std::string fname)
{
    StripeMapContent* stripeMap = stripeMapManager->GetStripeMapContent();
    return stripeMap->Dump(fname, iArrayinfo->GetName());
}

int
Mapper::WriteStripeMap(std::string fname)
{
    StripeMapContent* stripeMap = stripeMapManager->GetStripeMapContent();
    return stripeMap->DumpLoad(fname, iArrayinfo->GetName());
}

int
Mapper::ReadVsaMapEntry(int volId, BlkAddr rba, std::string fname)
{
    IVSAMap* iVSAMap = GetIVSAMap();
    VsaArray vsaArray;
    iVSAMap->GetVSAs(volId, rba, 1, vsaArray);
    VirtualBlkAddr vsa = vsaArray[0];

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
    int ret = vsaMapManager->GetVSAMapAPI()->SetVSAsInternal(volId, rba, blks);
    return ret;
}

int
Mapper::ReadStripeMapEntry(StripeId vsid, std::string fname)
{
    IStripeMap* iStripeMap = GetIStripeMap();
    StripeAddr entry = iStripeMap->GetLSA(vsid);

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
    IStripeMap* iStripeMap = GetIStripeMap();
    int ret = iStripeMap->SetLSA(vsid, lsid, loc);
    return ret;
}

int
Mapper::ReadReverseMap(StripeId vsid, std::string fname)
{
    ReverseMapPack* reverseMapPack = reverseMapManager->AllocReverseMapPack();

    int ret = _LoadReverseMapVsidFromMFS(reverseMapPack, vsid);
    if (ret < 0)
    {
        delete reverseMapPack;
        return ret;
    }

    // Store ReverseMap of vsid to Linux file(fname)
    MetaFileIntf* fileToStore = new MockFileIntf(fname, iArrayinfo->GetName());
    fileToStore->Create(reverseMapManager->GetReverseMapPerStripeFileSize());
    fileToStore->Open();

    ret = reverseMapPack->WbtFileSyncIo(fileToStore, MetaFsIoOpcode::Write);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "WbtFileIo() Write failed");
    }

    fileToStore->Close();
    delete fileToStore;
    delete reverseMapPack;
    return ret;
}

int
Mapper::WriteReverseMap(StripeId vsid, std::string fname)
{
    ReverseMapPack* reverseMapPack = reverseMapManager->AllocReverseMapPack();
    int ret = reverseMapPack->LinkVsid(vsid);
    if (ret < 0)
    {
        delete reverseMapPack;
        return ret;
    }

    // Load ReverseMap from Linux file(fname)
    MetaFileIntf* fileFromLoad = new MockFileIntf(fname, iArrayinfo->GetName());
    fileFromLoad->Open();

    ret = reverseMapPack->WbtFileSyncIo(fileFromLoad, MetaFsIoOpcode::Read);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "WbtFileIo() Read failed");
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
    uint64_t fileSize = reverseMapManager->GetWholeReverseMapFileSize();
    char* buffer = new char[fileSize]();

    // Load Whole ReverseMap from MFS
    ret = reverseMapManager->LoadWholeReverseMap(buffer);
    if (ret < 0)
    {
        delete[] buffer;
        return ret;
    }

    // Store Whole ReverseMap to Linux file(fname)
    MetaFileIntf* fileToStore = new MockFileIntf(fname, iArrayinfo->GetName());
    POS_TRACE_INFO((int)POS_EVENT_ID::REVMAP_FILE_SIZE, "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
                    reverseMapManager->GetReverseMapPerStripeFileSize(), addrInfo->maxVsid, fileSize);
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
    uint64_t fileSize = reverseMapManager->GetWholeReverseMapFileSize();
    char* buffer = new char[fileSize]();

    // Load Whole ReverseMap from Linux file(fname)
    MetaFileIntf* filefromLoad = new MockFileIntf(fname, iArrayinfo->GetName());
    filefromLoad->Open();
    filefromLoad->IssueIO(MetaFsIoOpcode::Read, 0, fileSize, buffer);
    filefromLoad->Close();
    delete filefromLoad;

    // Store Whole ReverseMap to MFS
    ret = reverseMapManager->StoreWholeReverseMap(buffer);

    delete[] buffer;
    return ret;
}

int
Mapper::ReadReverseMapEntry(StripeId vsid, BlkOffset offset, std::string fname)
{
    ReverseMapPack* reverseMapPack = reverseMapManager->AllocReverseMapPack();

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
    ReverseMapPack* reverseMapPack = reverseMapManager->AllocReverseMapPack();

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

    out << "Stripe Location: " << IN_WRITE_BUFFER_AREA << "(WRITE_BUFFER), " << IN_USER_AREA << "(USER_AREA) " << std::endl;
    out << "VSA map entry size: 0x" << std::hex << sizeof(VirtualBlkAddr) << std::endl;
    out << "Stripe map entry size: 0x" << std::hex << sizeof(StripeAddr) << std::endl;
    out << "VSA stripe id bit length: " << std::dec << STRIPE_ID_BIT_LEN << std::endl;
    out << "VSA block offset bit length: " << BLOCK_OFFSET_BIT_LEN << std::endl;
    out << "Stripe map location bit length: " << STRIPE_LOC_BIT_LEN << std::endl;
    out << "Stripe map stripe id bit length: " << STRIPE_ID_BIT_LEN << std::endl;

    if (stripeMapManager->GetStripeMapContent() == nullptr)
    {
        out << "Please create array and mount poseidonos to see stripe map mpage info" << std::endl;
    }
    else
    {
        out << "Meta page size: 0x" << std::hex << stripeMapManager->GetStripeMapContent()->GetPageSize() << std::endl;
        out << "Stripe map entries per mpage: 0x" << std::hex << stripeMapManager->GetStripeMapContent()->GetEntriesPerPage() << std::endl;

        VSAMapContent* validVsaMap = _GetFirstValidVolume();
        if (validVsaMap == nullptr)
        {
            out << "Please create volume to see volume map mpage info" << std::endl;
        }
        else
        {
            out << "VSA map entries per mpage: 0x" << std::hex << validVsaMap->GetEntriesPerPage() << std::endl;
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

void
Mapper::WaitForFlushAllMapsDone(void)
{
    while (vsaMapManager->AllMapsAsyncFlushed() == false || stripeMapManager->AllMapsAsyncFlushed() == false)
    {
        usleep(1);
    }
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "All VSAMaps and StripeMap are AsyncStored");
}

} // namespace pos
