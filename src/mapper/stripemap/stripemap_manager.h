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
#include "src/mapper/i_stripemap.h"
#include "src/mapper/stripemap/stripemap_content.h"
#include "src/mapper/address/mapper_address_info.h"

#include <atomic>
#include <map>
#include <string>

#include <pthread.h>

namespace pos
{
class EventScheduler;
class TelemetryPublisher;

class StripeMapManager : public IMapManagerInternal, public IStripeMap
{
public:
    StripeMapManager(void) = default;
    StripeMapManager(TelemetryPublisher* tp_, StripeMapContent* cont, EventScheduler* eventSched, MapperAddressInfo* info);
    StripeMapManager(TelemetryPublisher* tp_, EventScheduler* eventSched, MapperAddressInfo* info);
    virtual ~StripeMapManager(void);

    virtual int Init(void);
    virtual void Dispose(void);
    virtual int LoadStripeMapFile(void);
    virtual int FlushDirtyPagesGiven(MpageList list, EventSmartPtr cb);
    virtual int FlushTouchedPages(EventSmartPtr cb);
    virtual void MapFlushDone(int mapId) override;
    virtual void WaitAllPendingIoDone(void);
    virtual void WaitWritePendingIoDone(void);

    virtual StripeAddr GetLSA(StripeId vsid);
    virtual LsidRefResult GetLSAandReferLsid(StripeId vsid);
    virtual StripeId GetRandomLsid(StripeId vsid) override;
    virtual int SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc);
    virtual bool IsInUserDataArea(StripeAddr entry) { return entry.stripeLoc == IN_USER_AREA; }
    virtual bool IsInWriteBufferArea(StripeAddr entry) { return entry.stripeLoc == IN_WRITE_BUFFER_AREA; }
    virtual MpageList GetDirtyStripeMapPages(int vsid);

    virtual StripeMapContent* GetStripeMapContent(void);
    virtual void SetStripeMapContent(StripeMapContent* content);

    virtual int Dump(std::string fileName);
    virtual int DumpLoad(std::string fileName);

private:
    int _FlushMap(void);
    void _MapLoadDone(int param);
    void _WaitLoadIoDone(void);

    StripeMapContent* stripeMap;
    pthread_rwlock_t stripeMapLock;
    MapperAddressInfo* addrInfo;
    std::atomic<int> numLoadIssuedCount;
    std::atomic<int> numWriteIssuedCount;

    EventSmartPtr callback;
    EventScheduler* eventScheduler;
    TelemetryPublisher* tp;
};

} // namespace pos
