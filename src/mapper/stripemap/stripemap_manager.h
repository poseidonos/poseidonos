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

class StripeMapManager : public IMapManagerInternal, public IStripeMap
{
public:
    StripeMapManager(MapperAddressInfo* info, std::string arrayName, int arrayId);
    virtual ~StripeMapManager(void);

    int Init(void);
    void Dispose(void);
    int LoadStripeMapFile(void);
    int FlushMap(void);
    void MapFlushDone(int mapId) override;
    void WaitAllPendingIoDone(void);
    void WaitWritePendingIoDone(void);


    StripeAddr GetLSA(StripeId vsid);
    LsidRefResult GetLSAandReferLsid(StripeId vsid);
    StripeId GetRandomLsid(StripeId vsid) override;
    int SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc);
    bool IsInUserDataArea(StripeAddr entry) { return entry.stripeLoc == IN_USER_AREA; }
    bool IsInWriteBufferArea(StripeAddr entry) { return entry.stripeLoc == IN_WRITE_BUFFER_AREA; }
    MpageList GetDirtyStripeMapPages(int vsid);

    StripeMapContent* GetStripeMapContent(void);
    bool AllMapsFlushedDone(void);

private:
    int _FlushMap(void);
    void _MapLoadDone(int param);
    void _WaitLoadIoDone(void);

    StripeMapContent* stripeMap;
    pthread_rwlock_t stripeMapLock;
    MapperAddressInfo* addrInfo;
    std::atomic<int> numLoadIssuedCount;
    std::atomic<int> numWriteIssuedCount;

    std::string arrayName;
    int arrayId;
};

} // namespace pos
