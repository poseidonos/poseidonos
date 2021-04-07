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

#undef Max
#include "../map_flush_handler.h"
#include "../mapper.h"
#include "gtest/gtest.h"
#include "src/lib/singleton.h"
#include "src/scheduler/event.h"
#include "src/volume/volume_list.h"
#include "volume_manager_mock.h"

#include <map>

namespace ibofos
{
const bool RET_BOOL_SUCCESS = true;
const int RET_INT_SUCCESS = 0;
const int TEST_VOL_ID = 0;
const uint64_t TEST_VOL_SIZE = 0x40000000; // 1GB
const uint64_t PAGE_SIZE = 4096;

const BlkAddr TEST_RBA_MAX = (TEST_VOL_SIZE / PAGE_SIZE) - 1;
const StripeId TEST_VSID_MAX = 1024 * 1024 - 1;
const BlkOffset TEST_OFFSET_MAX = 1024 * 1024 - 1;
const StripeId TEST_LSID_MAX = 1024 - 1;
const uint64_t SZ_1G = (1 << 30);
const int NUM_MAP_ENTRY = 128;

class LibForTest;

class MapperTest : public LibForTest, public ::testing::Test
{
public:
    int GetSavedVolumeSize(int volId, uint64_t& volSize);

protected:
    void SetUp() override;
    void TearDown() override;

    VSAMapContent*
    _GetVSAMap(int volId)
    {
        return mapperSUT->vsaMapManager->GetVSAMapContent(volId);
    }
    StripeMapContent*
    _GetStripeMap()
    {
        return mapperSUT->stripeMap;
    }
    MapHeader*
    _GetMapHeader(MapContent* map)
    {
        return &map->header;
    }
    MapIoHandler*
    _GetFlushHandler(MapContent* map)
    {
        return map->mapIoHandler;
    }
    bool
    _IsVSAMapLoaded(int volID)
    {
        return mapperSUT->vsaMapManager->IsVSAMapLoaded(volID);
    }
    int _GetVolSizeByLoadHeader(int volID, uint64_t& volSizeByte);
    BlkAddr _GetRbaMax(int volId);
    void _UseMockVolumeManager(void);
    void _DeleteMockVolumeManager(void);
    void _CreateVolume(int volId);
    void _LoadVolume(int volId);
    void _MountVolume(int volId);
    void _UnmountVolume(int volId);
    void _DeleteVolume(int volId);
    void _DeleteStripeMapFile(void);
    void _UpdateVSAMap(int volumeId, BlkAddr rba, StripeId vsidOrig,
        BlkOffset offsetOrig);
    void _CheckVSAMap(int volumeId, BlkAddr rba, StripeId vsidOrig,
        BlkOffset offsetOrig);
    void _UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc);
    void _CheckStripeMap(StripeId vsid, StripeId lsidOrig, StripeLoc locOrig);

    Mapper* mapperSUT;
    MockVolumeManager* mockVolumeManager;
    static std::map<int, uint64_t> createdVolumeInfo; // [volId, volSizeByte]

private:
    void _CreateRandVolume(int volId);
};

class MapFlushHandlerTest;

class MapFlushDoneEvent : public Event
{
public:
    MapFlushDoneEvent(int mapId, MapFlushHandlerTest* tester);
    virtual ~MapFlushDoneEvent(void);
    virtual bool Execute(void);

private:
    int mapId;
    MapFlushHandlerTest* tester;
};

class MapFlushHandlerTest : public MapperTest
{
public:
    void FlushDoneCallback(int mapId);

protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void _SimulateNPOR(void);
    void _SimulateSPOR(void);

    void _SetupMockEventScheduler(void);
    void _ResetMockEventScheduler(void);

    void _AddToDirtyPages(MpageList& dirtyPages, MpageList pageList);
    void _VSAMapDirtyUpdateTester(int volId, BlkAddr rba, StripeId vsidOrig,
        BlkOffset offsetOrig, MpageList& dirtyPages);
    void _StripeMapDirtyUpdateTester(StripeId vsid, StripeId lsid, StripeLoc loc,
        MpageList& dirtyPages);

    void _TriggerDirtyFlush(int mapId, MpageList& dirtyPages);
    void _TriggerFlush(int mapId);

    std::atomic<int> numMapsToFlush;
    std::atomic<int> numMapsFlushed;
    std::atomic<bool> flushCompleted;
};

} // namespace ibofos
