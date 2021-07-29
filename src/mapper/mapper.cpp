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
    addrInfo = new MapperAddressInfo(iarrayInfo);
    vsaMapManager = new VSAMapManager(addrInfo, iarrayInfo->GetName(), iarrayInfo->GetIndex());
    stripeMapManager = new StripeMapManager(addrInfo, iarrayInfo->GetName(), iarrayInfo->GetIndex());
    reverseMapManager = new ReverseMapManager(vsaMapManager->GetIVSAMap(), stripeMapManager, iArrayinfo);
    mapperWbt = new MapperWbt(addrInfo, vsaMapManager, stripeMapManager, reverseMapManager);
}

Mapper::~Mapper(void)
{
    delete mapperWbt;
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
        addrInfo->SetupAddressInfo();
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
        StoreAllMaps();
        Close();
        _UnregisterFromMapperService();
        isInitialized = false;
    }
}

void
Mapper::Shutdown(void)
{
    if (isInitialized == true)
    {
        Close();
        _UnregisterFromMapperService();
        isInitialized = false;
    }
}

void
Mapper::Flush(void)
{
    // no-op for IMountSequence
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
    return mapperWbt;
}

int
Mapper::FlushDirtyMpages(int mapId, EventSmartPtr event, MpageList dirtyPages)
{
    MapContent* map = _GetMapContent(mapId);
    if (nullptr == map)
    {
        return -EID(WRONG_MAP_ID);
    }

    // NVMe FLUSH command: executed by event (FlushCmdHandler)
    if (IMapFlush::DEFAULT_DIRTYPAGE_SET == dirtyPages)
    {
        return map->FlushTouchedPages(event);
    }
    // Journal Checkpoint: executed by event (CheckpointSubmission)
    else
    {
        return map->FlushDirtyPagesGiven(dirtyPages, event);
    }
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

    ret = vsaMapManager->StoreAllMaps();
    if (ret < 0)
    {
        return ret;
    }

    return ret;
}

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
    mapperService->RegisterMapper(iArrayinfo->GetName(), iArrayinfo->GetIndex(),
        GetIVSAMap(), GetIStripeMap(), GetIReverseMap(), GetIMapFlush(), GetIMapperWbt());
}

void
Mapper::_UnregisterFromMapperService(void)
{
    std::string arrayName = iArrayinfo->GetName();
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->UnregisterMapper(arrayName);
}

} // namespace pos
