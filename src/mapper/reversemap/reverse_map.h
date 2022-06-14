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
#include <string>
#include <tuple>
#include <vector>

#include "src/mapper/i_stripemap.h"
#include "src/mapper/vsamap/vsamap_manager.h"
#include "src/meta_file_intf/async_context.h"

namespace pos
{
const uint32_t REVMAP_MAGIC = 0xCAFEBABE;

const int TIME_AS_TEXT_LEN = 32;
const int REVMAP_ENTRY_SIZE = 12;
const int VOLUME_ID_BIT = 10;
const int DEFAULT_REVMAP_PAGE_SIZE = 4032;

const int REVMAP_SECTOR_SIZE = DEFAULT_REVMAP_PAGE_SIZE / 16; // 252
const int NUM_SECTOR_ENTRIES = DEFAULT_REVMAP_PAGE_SIZE /
    REVMAP_SECTOR_SIZE;                                         // 16
const int NUM_ENTRIES = REVMAP_SECTOR_SIZE / REVMAP_ENTRY_SIZE; // 21

class MetaFileIntf;
class Stripe;
class IVolumeInfoManager;
class TelemetryPublisher;

// The first page can have 21 * (16 - 1) == 315 entries
// since the second page, it has 21 * 16 = 336 entries

constexpr int
ReservedBit(int usedBit)
{
    return REVMAP_ENTRY_SIZE * 8 - usedBit;
}

enum class ACTION
{
    OPEN,
    CLOSE,
};

enum IoDirection
{
    IO_FLUSH = 1,
    IO_LOAD,
};

struct RevMapEntry
{
    union
    {
        uint8_t byte[REVMAP_ENTRY_SIZE];

        struct __attribute__((packed))
        {
            BlkAddr rba;                                                         // 8 B
            uint32_t volumeId : VOLUME_ID_BIT;                                   // 10 b
            uint32_t reserved : ReservedBit(BLOCK_ADDR_BIT_LEN + VOLUME_ID_BIT); // 22 b
        } entry;
    } u;
};

struct RevMapSector
{
    union
    {
        uint8_t byte[REVMAP_SECTOR_SIZE];

        struct __attribute__((packed))
        {
            uint32_t magic;    // 4
            StripeId vsid;     // 4
            uint32_t volumeId; // 4
            StripeId wbLsid;   // 4

            uint32_t openSec;                // 4
            uint32_t openNsec;               // 4
            char openTime[TIME_AS_TEXT_LEN]; // 32

            uint32_t closeSec;                // 4
            uint32_t closeNsec;               // 4
            char closeTime[TIME_AS_TEXT_LEN]; // 32
        } header;

        struct __attribute__((packed))
        {
            RevMapEntry entry[NUM_ENTRIES];
        } body;
    } u;
};

struct RevMap
{
    RevMapSector sector[NUM_SECTOR_ENTRIES];
};

class RevMapPageAsyncIoCtx : public AsyncMetaFileIoCtx
{
public:
    int mpageNum;
    Stripe* stripeToFlush = nullptr;
};

class ReverseMapPack
{
    friend class ReverseMapTest;

public:
    ReverseMapPack(void);
    virtual ~ReverseMapPack(void);

    virtual void Init(MetaFileIntf* file, StripeId wbLsid_, StripeId vsid_, uint32_t mpageSize_, uint32_t numMpagesPerStripe_, TelemetryPublisher* tp);
    virtual void Assign(StripeId vsid);

    virtual int Load(uint64_t fileOffset, EventSmartPtr cb, uint32_t vsid);
    virtual int Flush(Stripe* stripe, uint64_t fileOffset, EventSmartPtr cb, uint32_t vsid);

    virtual int SetReverseMapEntry(uint64_t offset, BlkAddr rba, uint32_t volumeId);
    virtual std::tuple<BlkAddr, uint32_t> GetReverseMapEntry(uint64_t offset);
    virtual char* GetRevMapPtrForWBT(void) { return reinterpret_cast<char*>(&revMaps[0]->sector[0]); }
    virtual void WaitPendingIoDone(void);

private:
    void _SetHeader(StripeId wblsid, StripeId vsid);
    std::tuple<uint32_t, uint32_t, uint32_t> _ReverseMapGeometry(uint64_t offset);
    void _RevMapPageIoDone(AsyncMetaFileIoCtx* ctx);

    StripeId wbLsid;
    StripeId vsid;   // SSD LSID
    std::vector<RevMap*> revMaps;

    MetaFileIntf* revMapfile; // MFS file
    uint32_t mpageSize;
    std::atomic<uint32_t> mfsAsyncIoDonePages;
    std::atomic<MapFlushState> mapFlushState;
    uint64_t numMpagesPerStripe; // It depends on block count per a stripe
    int ioError; // indicates if there is an Async-IO error among mpages
    int ioDirection;
    EventSmartPtr callback;
    TelemetryPublisher* telemetryPublisher;
    std::atomic<uint64_t> issuedIoCnt;
};

} // namespace pos
