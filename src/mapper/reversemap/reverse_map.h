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

#include <tuple>
#include <vector>
#include <string>

#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
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

    void Init(uint64_t mpsize, uint64_t nmpPerStripe, MetaFileIntf* file, std::string arrName);
    void Init(StripeId wblsid, IVSAMap* ivsaMap, IStripeMap* istripeMap);
    int LinkVsid(StripeId vsid); // vsid == SSD LSID
    int UnLinkVsid(void);

    int Load(EventSmartPtr callback);
    int Flush(Stripe* stripe, EventSmartPtr callback);

    int SetReverseMapEntry(uint32_t offset, BlkAddr rba, uint32_t volumeId);
    int ReconstructMap(uint32_t volumeId, StripeId vsid, StripeId lsid, uint64_t blockCount);
    std::tuple<BlkAddr, uint32_t> GetReverseMapEntry(uint32_t offset);
    int IsAsyncIoDone(void);
    int GetIoError(void);
    int WbtFileSyncIo(MetaFileIntf* fileLinux, MetaFsIoOpcode IoDirection);

private:
    void _HeaderInit(StripeId wblsid);
    int _SetTimeToHeader(ACTION act);
    std::tuple<uint32_t, uint32_t, uint32_t> _ReverseMapGeometry(uint64_t offset);
    std::tuple<uint32_t, uint32_t> _GetCurrentTime(void);
    void _RevMapPageIoDone(AsyncMetaFileIoCtx* ctx);
    bool _FindRba(uint64_t blockOffset, BlkAddr rbaStart, BlkAddr& foundRba);

    bool linkedToVsid;
    StripeId vsid;   // SSD LSID
    StripeId wbLsid; // WriteBuffer LSID

    uint64_t mpageSize;          // Optimal page size for each FS (MFS, legacy)
    uint64_t numMpagesPerStripe; // It depends on block count per a stripe
    uint64_t fileSizePerStripe;

    MetaFileIntf* revMapfile; // MFS file
    std::atomic<uint32_t> mfsAsyncIoDonePages;
    int ioError; // indicates if there is an Async-IO error among mpages
    int ioDirection;
    std::vector<RevMap*> revMaps;
    EventSmartPtr callback;

    IVSAMap* iVSAMap;
    IStripeMap* iStripeMap;
    std::string arrayName;

    struct ReconstructInfo
    {
        StripeId lsid;
        StripeId vsid;
        uint32_t volId;
        BlkAddr totalRbaNum;

        int Init(StripeId lsid, StripeId vsid, uint32_t volumeId, std::string arrName);
    };
    ReconstructInfo reconstructStatus;
};

} // namespace pos
