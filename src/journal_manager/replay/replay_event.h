#pragma once

#include <vector>

#include "../log/log_event.h"

namespace ibofos
{
class Mapper;
class Allocator;

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
    ReplayEvent(void);
    virtual ~ReplayEvent(void);
    virtual int Replay(void) = 0;
    virtual ReplayEventType GetType(void) = 0;
};

class ReplayBlockMapUpdate : public ReplayEvent
{
public:
    ReplayBlockMapUpdate(Mapper* mapper, Allocator* allocator, BlockWriteDoneLog dat);
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

    bool _InvalidateOldBlock(uint32_t offset);
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

    Mapper* mapper;
    Allocator* allocator;

    BlockWriteDoneLog logData;
    std::vector<VirtualBlkAddr> readMap;
};

class ReplayStripeMapUpdate : public ReplayEvent
{
public:
    ReplayStripeMapUpdate(Mapper* mapper, StripeMapUpdatedLog dat);
    virtual ~ReplayStripeMapUpdate(void);

    virtual int Replay(void) override;
    inline StripeMapUpdatedLog
    GetLog(void)
    {
        return logData;
    }

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::STRIPE_MAP_UPDATE;
    }

private:
    Mapper* mapper;
    StripeMapUpdatedLog logData;
};

class ReplayStripeAllocation : public ReplayEvent
{
public:
    ReplayStripeAllocation(Mapper* mapper, Allocator* allocator, StripeId vsid, StripeId wbLsid);
    virtual ~ReplayStripeAllocation(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::STRIPE_ALLOCATION;
    }

private:
    Mapper* mapper;
    Allocator* allocator;
    StripeId vsid;
    StripeId wbLsid;
};

class ReplaySegmentAllocation : public ReplayEvent
{
public:
    ReplaySegmentAllocation(Allocator* allocator, StripeId userLsid);
    virtual ~ReplaySegmentAllocation(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::SEGMENT_ALLOCATION;
    }

private:
    Allocator* allocator;
    StripeId userLsid;
};

class ReplayStripeFlush : public ReplayEvent
{
public:
    ReplayStripeFlush(Allocator* allocator, StripeId vsid, StripeId wbLsid, StripeId userLsid);
    virtual ~ReplayStripeFlush(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::STRIPE_FLUSH;
    }

private:
    Allocator* allocator;
    StripeId vsid;
    StripeId wbLsid;
    StripeId userLsid;
};

} // namespace ibofos
