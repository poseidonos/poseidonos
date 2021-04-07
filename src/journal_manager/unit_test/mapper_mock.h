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

#undef Max
#include "gmock/gmock.h"
#include "src/mapper/mapper.h"
#include "stripe_test_info.h"
#include "test_info.h"

using namespace ibofos;

class MockMapFlushHandler
{
public:
    explicit MockMapFlushHandler(int mapId);
    int StartDirtyPageFlush(MpageList dirtyPages, EventSmartPtr callback);

private:
    void _MpageFlushed(int pageId);

    int id;
    std::mutex lock;
    std::atomic<int> numPagesToFlush;
    std::atomic<int> numPagesFlushed;
    EventSmartPtr callbackEvent;
};

class MockMapper : public Mapper
{
public:
    explicit MockMapper(TestInfo* _testInfo);
    virtual ~MockMapper(void);

    MpageList GetVsaMapDirtyPages(int volId, BlkAddr rba, uint32_t numBlks);
    MpageList GetStripeMapDirtyPages(StripeId vsid);

    MOCK_METHOD(int, UpdateStripeMap,
        (StripeId vsid, StripeId lsid, StripeLoc loc), (override));
    MOCK_METHOD(StripeAddr, GetLSA,
        (StripeId vsid), (override));
    MOCK_METHOD(int, StartDirtyPageFlush,
        (int mapId, MpageList dirtyPages, EventSmartPtr callback), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAInternal,
        (int volumeId, BlkAddr rba, int& caller), (override));
    MOCK_METHOD(int, SetVsaMapInternal,
        (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));

private:
    void _Init(void);
    int _StartDirtyPageFlush(int mapId, MpageList dirtyPages, EventSmartPtr callback);
    int _UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc);
    StripeAddr _GetLsid(StripeId vsid);
    VirtualBlkAddr _GetVSAInternal(int volumeId, BlkAddr rba, int& caller);
    int _SetVsaMapInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);

    TestInfo* testInfo;
    std::vector<MockMapFlushHandler*> flushHandler;
    MockMapFlushHandler* stripeMapFlushHandler;
    StripeAddr* stripeMap;
    VirtualBlkAddr** vsaMap;
};
