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

#pragma once

#include <array>
#include <string>
#include <tuple>

#include "mapper_address_info.h"
#include "reverse_map.h"
#include "src/lib/singleton.h"
#include "stripemap_content.h"
#include "vsamap_content.h"
#include "vsamap_manager.h"

class MapperTest;

namespace ibofos
{
using VsaArray = std::array<VirtualBlkAddr, MAX_PROCESSABLE_BLOCK_COUNT>;
const uint64_t INVALID_BLOCK_MAP_SIZE = 0xFFFFFFFFFFFFFFFF;
using LsidRefResult = std::tuple<StripeAddr, bool>;

class Mapper
{
    friend class MapperTest;

public:
    Mapper(void);
    virtual ~Mapper(void);

    void Init(void);
    int SyncStore(void);  // FlushMetadata::Start()
    int AsyncStore(void); // MainHandler::Unmount()
    void Close(void);

    // Get & Set APIs for VSAMap
    int GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray);
    VirtualBlkAddr GetVSA(int volumeId, BlkAddr rba);
    virtual VirtualBlkAddr GetVSAInternal(int volumeId, BlkAddr rba, int& caller);
    VirtualBlkAddr GetRandomVSA(BlkAddr rba);
    int SetVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);
    virtual int SetVsaMapInternal(int volumeId, BlkAddr startRba,
        VirtualBlks& virtualBlks);
    int ResetVSARange(int volumeId, BlkAddr rba, uint64_t cnt);

    // Get & Set APIs for StripeMap
    virtual StripeAddr GetLSA(StripeId vsid);
    LsidRefResult GetAndReferLsid(StripeId vsid);
    StripeId GetRandomLsid(StripeId vsid);
    virtual int UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc);
    bool ReferLsid(StripeAddr& lsidEntry);
    void DereferLsid(StripeAddr& lsidEntry, uint32_t blockCount);

    // ReverseMap
    int LinkReverseMap(Stripe* stripe, StripeId wbLsid, StripeId vsid);
    MetaFileIntf* GetRevMapWholeFile(void);

    // CheckPoint
    MpageList GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks);
    MpageList GetDirtyStripeMapPages(int vsid);
    virtual int StartDirtyPageFlush(int mapId, MpageList dirtyPages, EventSmartPtr callback);

    // NVMe FLUSH command
    int FlushMap(int mapId, EventSmartPtr callback);

    // WBT
    int GetMapLayout(std::string fname);
    int ReadVsaMap(int volId, std::string fname);
    int ReadVsaMapEntry(int volId, BlkAddr rba, std::string fname);
    int WriteVsaMap(int volId, std::string fname);
    int WriteVsaMapEntry(int volId, BlkAddr rba, VirtualBlkAddr vsa);
    int ReadStripeMap(std::string fname);
    int ReadStripeMapEntry(StripeId vsid, std::string fname);
    int WriteStripeMap(std::string fname);
    int WriteStripeMapEntry(StripeId vsid, StripeLoc loc, StripeId lsid);
    int ReadReverseMap(StripeId vsid, std::string fname);
    int ReadWholeReverseMap(std::string fname);
    int ReadReverseMapEntry(StripeId vsid, BlkOffset offset, std::string fname);
    int WriteReverseMap(StripeId vsid, std::string fname);
    int WriteWholeReverseMap(std::string fname);
    int WriteReverseMapEntry(StripeId vsid, BlkOffset offset, BlkAddr rba,
        uint32_t volumeId);

    // Etc
    int64_t GetNumUsedBlocks(int volId);
    void RegisterToPublisher(void);
    void RemoveFromPublisher(void);
    bool
    IsInUserDataArea(StripeAddr entry)
    {
        return entry.stripeLoc == IN_USER_AREA;
    }
    bool
    IsInWriteBufferArea(StripeAddr entry)
    {
        return entry.stripeLoc == IN_WRITE_BUFFER_AREA;
    }
    void CheckMapStoreDone(void);

private:
    int _UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);
    VirtualBlkAddr _ReadVSA(int volumeId, BlkAddr rba);
    void _InitMetadata(const MapperAddressInfo& info);
    VSAMapContent* _GetFirstValidVolume(void);
    MapContent* _GetMapContent(int mapId);
    int _LoadReverseMapVsidFromMFS(ReverseMapPack* reverseMapPack, StripeId vsid);
    int _StoreReverseMapToMFS(ReverseMapPack* reverseMapPack);

    MapperAddressInfo info;
    VSAMapManager* vsaMapManager;
    StripeMapContent* stripeMap;
    ReverseMapPack* revMapPacks;
    MetaFileIntf* revMapWholefile;

    pthread_rwlock_t stripeMapLock;
};

using MapperSingleton = Singleton<Mapper>;

} // namespace ibofos
