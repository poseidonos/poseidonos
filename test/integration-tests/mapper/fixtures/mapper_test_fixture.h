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

#include "src/allocator/allocator.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/mapper/mapper.h"
#include "src/mapper/vsamap/vsamap_manager.h"
#include "src/volume/i_volume_info_manager.h"

#include "test/integration-tests/mapper/utils/random_for_it.h"
#include "test/integration-tests/mapper/test_doubles/array_info_mock.h"
#include "test/integration-tests/mapper/test_doubles/volume_manager_mock.h"

#include "gtest/gtest.h"

#include <map>
#include <vector>

namespace pos
{

class MapperTestFixture : public RandomForUT, public ::testing::Test
{
public:
    int GetSavedVolumeSize(int volId, uint64_t& volSize);
    void FlushDoneCallback(int mapId);

protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void _UseMockVolumeManager(void);
    void _DeleteMockVolumeManager(void);

    void _CreateRandVolume(int volId);
    void _LoadVolume(int volId);
    void _MountVolume(int volId);
    void _UnmountVolume(int volId);
    void _DeleteVolume(int volId);

    BlkAddr _GetRbaMax(int volId);
    void _SetVSAs(int volumeId, BlkAddr rba, StripeId vsidOrig, BlkOffset offsetOrig);
    void _GetAndCompareVSA(int volumeId, BlkAddr rba, StripeId vsidOrig, BlkOffset offsetOrig);
    void _SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc);
    void _GetAndCompareLSA(StripeId vsid, StripeId lsidOrig, StripeLoc locOrig);

    void _SetupMockEventScheduler(void);
    void _ResetMockEventScheduler(void);
    void _SimulateNPOR(void);
    void _SimulateSPOR(void);
    void _VSAMapDirtyUpdateTester(int volId, BlkAddr rba, StripeId vsidOrig, BlkOffset offsetOrig, MpageList& dirtyPages);
    void _StripeMapDirtyUpdateTester(StripeId vsid, StripeId lsid, StripeLoc loc, MpageList& dirtyPages);
    void _AddToDirtyPages(MpageList& dirtyPages, MpageList pageList);
    void _FlushDirtyPagesGiven(int mapId, MpageList& dirtyPages);
    void _FlushTouchedPages(int mapId);

    static std::map<int, uint64_t> createdVolumeInfo;   // [volId, volSizeByte]
    static std::vector<uint32_t> rbas0, vsids0, offsets0;
    static std::vector<uint32_t> rbas1, vsids1, offsets1;

    std::atomic<int> numMapsToFlush;
    std::atomic<int> numMapsFlushed;
    std::atomic<bool> flushCompleted;

    Mapper* mapperSUT;
    VSAMapManager* vsaMapManagerSUT;
    StripeMapManager* stripeMapManagerSUT;
    ReverseMapManager* reverseMapManagerSUT;
    IVolumeInfoManager* volManager;

    Allocator* allocator;
    MockVolumeManager* mockVolumeManager;
    MockArrayInfo* mockArrayInfo;
    EventScheduler* mockEventScheduler;

    MapperAddressInfo* addrInfo;
};

}   // namespace pos
