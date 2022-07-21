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

#include <map>
#include <tuple>

#include "src/array_models/interface/i_array_info.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/i_reversemap.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/volume/i_volume_info_manager.h"

namespace pos
{
class TelemetryPublisher;

class ReverseMapManager : public IReverseMap
{
public:
    ReverseMapManager(void) = default;
    ReverseMapManager(IVSAMap* ivsaMap, IStripeMap* istripeMap, IVolumeInfoManager* vol, MapperAddressInfo* addrInfo_, TelemetryPublisher* tp);
    virtual ~ReverseMapManager(void);

    virtual void Init(void);
    virtual void Dispose(void);
    virtual int Load(ReverseMapPack* rev, StripeId wblsid, StripeId vsid, EventSmartPtr cb);
    virtual int Flush(ReverseMapPack* rev, StripeId wblsid, Stripe* stripe, StripeId vsid, EventSmartPtr cb);

    virtual int UpdateReverseMapEntry(ReverseMapPack* rev, StripeId wblsid, uint64_t offset, BlkAddr rba, uint32_t volumeId);
    virtual std::tuple<BlkAddr, uint32_t> GetReverseMapEntry(ReverseMapPack* rev, StripeId wblsid, uint64_t offset);
    virtual ReverseMapPack* Assign(StripeId wblsid, StripeId vsid);
    virtual ReverseMapPack* AllocReverseMapPack(uint32_t vsid);
    virtual int ReconstructReverseMap(uint32_t volumeId, uint64_t totalRba, uint32_t wblsid, uint32_t vsid, uint64_t blockCount, std::map<uint64_t, BlkAddr> revMapInfos);

    virtual uint64_t GetReverseMapPerStripeFileSize(void);
    virtual uint64_t GetWholeReverseMapFileSize(void);
    virtual int LoadReverseMapForWBT(MetaFileIntf* fileLinux, uint64_t offset, uint64_t fileSize, char* buf);
    virtual int StoreReverseMapForWBT(MetaFileIntf* fileLinux, uint64_t offset, uint64_t fileSize, char* buf);
    virtual char* GetReverseMapPtrForWBT(void);
    virtual void WaitAllPendingIoDone(void);

private:
    bool _FindRba(uint32_t volumeId, uint64_t totalRbaNum, StripeId vsid, StripeId wblsid, uint64_t offset, BlkAddr rbaStart, BlkAddr& foundRba);
    int _SetNumMpages(void);

    uint64_t mpageSize;          // Optimal page size for each FS (MFS, legacy)
    uint64_t numMpagesPerStripe; // It depends on block count per a stripe
    uint64_t fileSizePerStripe;
    uint64_t fileSizeWholeRevermap;

    ReverseMapPack* revMapPacks;
    MetaFileIntf* revMapWholefile;

    IVSAMap* iVSAMap;
    IStripeMap* iStripeMap;
    IVolumeInfoManager* volumeManager;
    MapperAddressInfo* addrInfo;
    TelemetryPublisher* telemetryPublisher;
    bool rocksDbEnabled;
};

} // namespace pos
