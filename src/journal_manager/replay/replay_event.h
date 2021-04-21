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

#include <vector>

#include "../log/log_event.h"

namespace pos
{
class IVSAMap;
class IStripeMap;
class IBlockAllocator;
class IWBStripeCtx;
class ISegmentCtx;
class IArrayInfo;
class StripeReplayStatus;

enum class ReplayEventType
{
    BLOCK_MAP_UPDATE,
    STRIPE_MAP_UPDATE,
    STRIPE_ALLOCATION,
    SEGMENT_ALLOCATION,
    STRIPE_FLUSH
};

class ReplayEvent
{
public:
    explicit ReplayEvent(StripeReplayStatus* status);
    virtual ~ReplayEvent(void);

    virtual int Replay(void) = 0;
    virtual ReplayEventType GetType(void) = 0;

protected:
    StripeReplayStatus* status;
};

class ReplayBlockMapUpdate : public ReplayEvent
{
public:
    ReplayBlockMapUpdate(IVSAMap* ivsaMa, IBlockAllocator* iblkAllocator,
        StripeReplayStatus* status, BlockWriteDoneLog dat);
    virtual ~ReplayBlockMapUpdate(void);

    virtual int Replay(void) override;
    inline BlockWriteDoneLog
    GetLog(void)
    {
        return logData;
    }

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::BLOCK_MAP_UPDATE;
    }

private:
    void _ReadBlockMap(void);

    void _InvalidateOldBlock(uint32_t offset);
    int _UpdateMap(uint32_t offset);

    inline VirtualBlkAddr
    _GetVsa(uint32_t offset)
    {
        VirtualBlkAddr vsa = {
            .stripeId = logData.startVsa.stripeId,
            .offset = logData.startVsa.offset + offset};
        return vsa;
    }

    inline VirtualBlkAddr
    _GetOldVsa(uint32_t offset)
    {
        VirtualBlkAddr oldVsa = {
            .stripeId = logData.oldVsa.stripeId,
            .offset = logData.oldVsa.offset + offset};
        return oldVsa;
    }

    inline BlkAddr
    _GetRba(uint32_t offset)
    {
        return logData.startRba + offset;
    }

    IVSAMap* vsaMap;
    IBlockAllocator* blockAllocator;

    BlockWriteDoneLog logData;
    std::vector<VirtualBlkAddr> readMap;
};

class ReplayStripeMapUpdate : public ReplayEvent
{
public:
    ReplayStripeMapUpdate(IStripeMap* stripeMap, StripeReplayStatus* status,
        StripeId vsid, StripeAddr dest);
    virtual ~ReplayStripeMapUpdate(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::STRIPE_MAP_UPDATE;
    }

private:
    IStripeMap* stripeMap;

    StripeId vsid;
    StripeAddr dest;
};

class ReplayStripeAllocation : public ReplayEvent
{
public:
    ReplayStripeAllocation(IStripeMap* istripeMap, IWBStripeCtx* iwbStripeCtx,
        StripeReplayStatus* status, StripeId vsid, StripeId wbLsid);
    virtual ~ReplayStripeAllocation(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::STRIPE_ALLOCATION;
    }

private:
    IStripeMap* stripeMap;
    IWBStripeCtx* wbStripeCtx;

    StripeId vsid;
    StripeId wbLsid;
};

class ReplaySegmentAllocation : public ReplayEvent
{
public:
    ReplaySegmentAllocation(ISegmentCtx* isegCtx, IArrayInfo* arrayInfo,
        StripeReplayStatus* status, StripeId stripeId);
    virtual ~ReplaySegmentAllocation(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::SEGMENT_ALLOCATION;
    }

private:
    ISegmentCtx* segmentCtx;
    IArrayInfo* arrayInfo;

    StripeId userLsid;
};

class ReplayStripeFlush : public ReplayEvent
{
public:
    ReplayStripeFlush(IWBStripeCtx* iwbStripeCtx, ISegmentCtx* isegCtx,
        StripeReplayStatus* status, StripeId vsid, StripeId wbLsid, StripeId userLsid);
    virtual ~ReplayStripeFlush(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::STRIPE_FLUSH;
    }

    StripeId
    GetVsid(void)
    {
        return vsid;
    }

    StripeId
    GetWbLsid(void)
    {
        return wbLsid;
    }

    StripeId
    GetUserLsid(void)
    {
        return userLsid;
    }

private:
    IWBStripeCtx* wbStripeCtx;
    ISegmentCtx* segmentCtx;

    StripeId vsid;
    StripeId wbLsid;
    StripeId userLsid;
};

} // namespace pos
