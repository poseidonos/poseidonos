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

#include <string>
#include <vector>

#include "src/array_models/interface/i_mount_sequence.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/i_map_flush.h"
#include "src/mapper/i_mapper_volume_event_handler.h"
#include "src/mapper/i_mapper_wbt.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/mapper_wbt.h"
#include "src/mapper/reversemap/reversemap_manager.h"
#include "src/mapper/stripemap/stripemap_manager.h"
#include "src/mapper/vsamap/vsamap_manager.h"
#include "src/state/interface/i_state_control.h"
#include "src/metafs/metafs.h"

namespace pos
{
class IStateControl;
class MetaFs;
class TelemetryPublisher;
class TelemetryClient;

enum VolState
{
    NOT_EXIST,          // Not exist
    EXIST_UNLOADED,     // Unloaded
    BACKGROUND_MOUNTED, // Loaded
    FOREGROUND_MOUNTED, // Loaded
    VOLUME_LOADING,
    VOLUME_DELETING, // Deleting
    MAX_STATE
};

class VolumeMountState
{
public:
    VolumeMountState(void)
    {
        state = VolState::NOT_EXIST;
        volSize = 0;
    }
    ~VolumeMountState(void) {}
    VolState GetState(void)
    {
        return state;
    }
    void SetState(VolState state_)
    {
        state = state_;
    }
    void SetSize(uint64_t size)
    {
        volSize = size;
    }
    uint64_t GetSize(void)
    {
        return volSize;
    }
    std::mutex& GetVolStateLock(void)
    {
        return stateLock;
    }

    VolState state;
    uint64_t volSize;
    std::mutex stateLock;
};

class Mapper : public IMapFlush, public IMountSequence, public IMapperVolumeEventHandler, public IVSAMap
{
public:
    Mapper(TelemetryClient* tc_, TelemetryPublisher* tp, MapperWbt* mapperWbt_, VSAMapManager* vsaMapMan, StripeMapManager* stripeMan, ReverseMapManager* revMapMan, MapperAddressInfo* addrInfo_, IArrayInfo* iarrayInfo, MetaFs* metaFs_);
    Mapper(IArrayInfo* iarrayInfo, MetaFs* metaFs_);
    virtual ~Mapper(void);

    virtual int Init(void);
    virtual void Dispose(void);
    virtual void Shutdown(void);
    virtual void Flush(void);

    virtual VSAMapManager* GetVSAMapManager(void) { return vsaMapManager; }
    virtual StripeMapManager* GetStripeMapManager(void) { return stripeMapManager; }
    virtual ReverseMapManager* GetReverseMapManager(void) { return reverseMapManager; }

    virtual IVSAMap* GetIVSAMap(void) { return this; }
    virtual IStripeMap* GetIStripeMap(void) { return stripeMapManager; }
    virtual IReverseMap* GetIReverseMap(void) { return reverseMapManager; }
    virtual IMapFlush* GetIMapFlush(void) { return this; }
    virtual IMapperWbt* GetIMapperWbt(void) { return mapperWbt; }
    virtual IMapperVolumeEventHandler* GetVolumeEventHandler(void) { return this; }

    virtual int VolumeCreated(int volId, uint64_t volSizeByte) override;
    virtual int VolumeMounted(int volId, uint64_t volSizeByte) override;
    virtual int VolumeLoaded(int volId, uint64_t volSizeByte) override;
    virtual int VolumeUnmounted(int volId, bool flushMapRequired) override;
    virtual int PrepareVolumeDelete(int volId) override;
    virtual int InvalidateAllBlocksTo(int volId, ISegmentCtx* segmentCtx) override;
    virtual int DeleteVolumeMap(int volumeId) override;
    virtual int VolumeDetached(vector<int> volList) override;

    virtual int GetVSAs(int volId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray);
    virtual int SetVSAs(int volId, BlkAddr startRba, VirtualBlks& virtualBlks);
    virtual VirtualBlkAddr GetRandomVSA(BlkAddr rba);
    virtual int64_t GetNumUsedBlks(int volId);
    virtual VirtualBlkAddr GetVSAInternal(int volId, BlkAddr rba, int& retry);
    virtual int SetVSAsInternal(int volId, BlkAddr startRba, VirtualBlks& virtualBlks);
    virtual VirtualBlkAddr GetVSAWithSyncOpen(int volId, BlkAddr rba);
    virtual int SetVSAsWithSyncOpen(int volId, BlkAddr startRba, VirtualBlks& virtualBlks);
    virtual MpageList GetDirtyVsaMapPages(int volId, BlkAddr startRba, uint64_t numBlks);

    virtual int EnableInternalAccess(int volId);
    virtual int FlushDirtyMpages(int mapId, EventSmartPtr callback);
    virtual int FlushDirtyMpagesGiven(int mapId, EventSmartPtr callback, MpageList dirtyPages);
    virtual int StoreAll(void);

    virtual void SetVolumeState(int volId, VolState state, uint64_t size); // for UT

private:
    void _Dispose(void);
    void _RegisterToMapperService(void);
    void _UnregisterFromMapperService(void);
    int _LoadVolumeMeta(int volId, bool delVol = false);
    void _ClearVolumeState(void);
    bool _ChangeVolumeStateDeleting(int volId);
    int _GetMpageSize(void);

    MapperAddressInfo* addrInfo;
    VSAMapManager* vsaMapManager;
    StripeMapManager* stripeMapManager;
    ReverseMapManager* reverseMapManager;
    MapperWbt* mapperWbt;
    MetaFs* metaFs;
    TelemetryPublisher* tp;
    TelemetryClient* tc;

    bool isInitialized;
    VolumeMountState volState[MAX_VOLUME_COUNT];
    std::atomic<uint32_t> numMapLoadedVol;
    std::atomic<uint32_t> numMountedVol;
    std::string arrayName;
};

} // namespace pos
