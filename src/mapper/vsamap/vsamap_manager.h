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

#pragma once

#include "src/mapper/include/mapper_const.h"
#include "src/mapper/i_map_manager.h"
#include "src/mapper/vsamap/vsamap_content.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/volume/volume_base.h"

#include <condition_variable>
#include <map>
#include <string>
#include <vector>

namespace pos
{
class EventScheduler;
class TelemetryPublisher;

class VSAMapManager : public IMapManagerInternal
{
public:
    VSAMapManager(void) = default;
    VSAMapManager(TelemetryPublisher* tp_, EventScheduler* eventSched, VSAMapContent* vsaMap, MapperAddressInfo* info);
    explicit VSAMapManager(TelemetryPublisher* tp_, MapperAddressInfo* info);
    virtual ~VSAMapManager(void);
    virtual int Init(void);
    virtual void Dispose(void);

    virtual int CreateVsaMapContent(VSAMapContent* vsaMap, int volId, uint64_t volSizeByte, bool delVol);
    virtual int LoadVSAMapFile(int volId);
    virtual int FlushDirtyPagesGiven(int volId, MpageList list, EventSmartPtr cb);
    virtual int FlushTouchedPages(int volId, EventSmartPtr cb);
    virtual int FlushAllMaps(void);
    virtual void WaitAllPendingIoDone(void);
    virtual void WaitLoadPendingIoDone(void);
    virtual void WaitWritePendingIoDone(void);
    virtual void WaitVolumePendingIoDone(int volId);
    virtual bool IsVolumeLoaded(int volId);
    virtual void MapFlushDone(int mapId);

    virtual int GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray);
    virtual int SetVSAs(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);
    virtual VirtualBlkAddr GetRandomVSA(BlkAddr rba);
    virtual int64_t GetNumUsedBlks(int volId);
    virtual VirtualBlkAddr GetVSAWoCond(int volumeId, BlkAddr rba);
    virtual int SetVSAsWoCond(int volumeId, BlkAddr startfRba, VirtualBlks& virtualBlks);
    virtual MpageList GetDirtyVsaMapPages(int volId, BlkAddr startRba, uint64_t numBlks);
    virtual VSAMapContent* GetVSAMapContent(int volId);
    virtual void SetVSAMapContent(int volId, VSAMapContent* content);

    virtual bool NeedToDeleteFile(int volId);
    virtual int InvalidateAllBlocks(int volId, ISegmentCtx* segmentCtx);
    virtual int DeleteVSAMap(int volId);

    virtual bool IsVsaMapAccessible(int volId);
    virtual void EnableVsaMapAccess(int volId);
    virtual void DisableVsaMapAccess(int volId);
    virtual bool IsVsaMapInternalAccesible(int volId);
    virtual void EnableVsaMapInternalAccess(int volId);
    virtual void DisableVsaMapInternalAccess(int volId);

    virtual int Dump(int volId, std::string fileName);
    virtual int DumpLoad(int volId, std::string fileName);

private:
    void _MapLoadDone(int volId);
    int _UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);

    MapperAddressInfo* addrInfo;
    VSAMapContent* vsaMaps[MAX_VOLUME_COUNT];
    std::atomic<MapFlushState> mapFlushState[MAX_VOLUME_COUNT];
    std::atomic<MapLoadState> mapLoadState[MAX_VOLUME_COUNT];
    std::atomic<bool> isVsaMapAccessable[MAX_VOLUME_COUNT];
    std::atomic<bool> isVsaMapInternalAccessable[MAX_VOLUME_COUNT];
    std::atomic<int> numWriteIssuedCount;
    std::atomic<int> numLoadIssuedCount;
    EventScheduler* eventScheduler;
    TelemetryPublisher* tp;
};

} // namespace pos
