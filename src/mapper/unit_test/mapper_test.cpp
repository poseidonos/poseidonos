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

#include "mapper_test.h"

#include "lib_test.h"
#include "src/allocator/stripe.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
// Global variables
std::map<int, uint64_t> MapperTest::createdVolumeInfo;
std::vector<uint32_t> rbas0, vsids0, offsets0, lsids0;
std::vector<uint32_t> rbas1, vsids1, offsets1, lsids1;

void
MapperTest::SetUp(void)
{
    mapperSUT = MapperSingleton::Instance();
    mapperSUT->Init();
}

void
MapperTest::TearDown(void)
{
    MapperSingleton::ResetInstance();
}

int
MapperTest::GetSavedVolumeSize(int volId, uint64_t& volSize)
{
    volSize = createdVolumeInfo[volId];
    return 0;
}

void
MapperTest::_CreateRandVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    uint64_t volSizeByte = GetRandomSection(1, 50) * SZ_1G; // 1 ... 50 GB
    IBOF_TRACE_WARN(9999, "Volume Name:{}  Size(Byte):{}", volName, volSizeByte);

    bool reb = mapperSUT->vsaMapManager->VolumeCreated(volName, volId, volSizeByte, 0, 0);
    EXPECT_EQ(reb, true);
    createdVolumeInfo.emplace(volId, volSizeByte);
}

void
MapperTest::_UseMockVolumeManager(void)
{
    mockVolumeManager = new MockVolumeManager();
    EXPECT_CALL(*mockVolumeManager, GetVolumeSize(_, _)).WillRepeatedly(Invoke(this, &MapperTest::GetSavedVolumeSize));
    mapperSUT->vsaMapManager->SetVolumeManagerObject(mockVolumeManager);
}

void
MapperTest::_DeleteMockVolumeManager(void)
{
    delete mockVolumeManager;
    mockVolumeManager = nullptr;
}

BlkAddr
MapperTest::_GetRbaMax(int volId)
{
    return (createdVolumeInfo[volId] / PAGE_SIZE) - 1;
}

void
MapperTest::_CreateVolume(int volId)
{
    _CreateRandVolume(volId);
}

void
MapperTest::_LoadVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    bool retb = mapperSUT->vsaMapManager->VolumeLoaded(volName, volId, 0, 0, 0);
    EXPECT_EQ(retb, true);
}

void
MapperTest::_MountVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    bool retb = mapperSUT->vsaMapManager->VolumeMounted(volName, "", volId, createdVolumeInfo[volId], 0, 0);
    EXPECT_EQ(retb, true);
}

void
MapperTest::_UnmountVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    bool retb = mapperSUT->vsaMapManager->VolumeUnmounted(volName, volId);
    EXPECT_EQ(retb, true);
}

void
MapperTest::_DeleteVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    bool retb = mapperSUT->vsaMapManager->VolumeDeleted(volName, volId, createdVolumeInfo[volId]);
    EXPECT_EQ(retb, true);
    createdVolumeInfo.erase(volId);
}

void
MapperTest::_DeleteStripeMapFile(void)
{
    int reti = 0;
    StripeMapContent* stripemap = _GetStripeMap();

    reti = stripemap->DeleteMapFile();
    EXPECT_EQ(reti, RET_INT_SUCCESS);
}

void
MapperTest::_UpdateVSAMap(int volumeId, BlkAddr rba, StripeId vsidOrig,
    BlkOffset offsetOrig)
{
    VirtualBlkAddr vsaOrig = {.stripeId = vsidOrig, .offset = offsetOrig};
    VirtualBlks vb = {.startVsa = vsaOrig, .numBlks = 1};
    int reti = mapperSUT->UpdateVsaMap(volumeId, rba, vb);
    EXPECT_EQ(reti, RET_INT_SUCCESS);
}

void
MapperTest::_CheckVSAMap(int volumeId, BlkAddr rba, StripeId vsidOrig,
    BlkOffset offsetOrig)
{
    VirtualBlkAddr vsa = mapperSUT->GetVSA(volumeId, rba);
    VirtualBlkAddr vsaOrig = {.stripeId = vsidOrig, .offset = offsetOrig};
    EXPECT_EQ(vsa, vsaOrig);
}

void
MapperTest::_UpdateStripeMap(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    int reti = mapperSUT->UpdateStripeMap(vsid, lsid, loc);
    EXPECT_EQ(reti, RET_INT_SUCCESS);
}

void
MapperTest::_CheckStripeMap(StripeId vsid, StripeId lsidOrig, StripeLoc locOrig)
{
    StripeAddr lsaOrig = {.stripeLoc = locOrig, .stripeId = lsidOrig};
    StripeAddr lsa = mapperSUT->GetLSA(vsid);
    EXPECT_EQ(lsa, lsaOrig);
}

int
MapperTest::_GetVolSizeByLoadHeader(int volID, uint64_t& volSizeByte)
{
    VSAMapContent* vsaMap = _GetVSAMap(volID);
    return vsaMap->GetNumMpageByLoadHeader(volID, volSizeByte);
}

TEST_F(MapperTest, GetMapLayout)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.GetMapLayout ********************");
    _CreateVolume(0);
    int reti = mapperSUT->GetMapLayout("get_map_layout.txt");
    EXPECT_EQ(reti, RET_INT_SUCCESS);
    _DeleteVolume(0);
}

TEST_F(MapperTest, SetAndStore)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.SetAndStore ********************");
    int reti = 0;
    _CreateVolume(0);
    _MountVolume(0);

    FillRandomSection(0, _GetRbaMax(0), rbas0, NUM_MAP_ENTRY);
    PrintData("RBA   ", rbas0);
    FillRandomSection(0, TEST_VSID_MAX, vsids0, NUM_MAP_ENTRY);
    PrintData("VSID  ", vsids0);
    FillRandomSection(0, TEST_OFFSET_MAX, offsets0, NUM_MAP_ENTRY);
    PrintData("OFFSET", offsets0);
    FillRandomSection(0, TEST_LSID_MAX, lsids0, NUM_MAP_ENTRY);
    PrintData("LSID  ", lsids0);
    for (int i = 0; i < NUM_MAP_ENTRY; ++i)
    {
        _UpdateVSAMap(0, rbas0[i], vsids0[i], offsets0[i]);
        _UpdateStripeMap(vsids0[i], lsids0[i], IN_WRITE_BUFFER_AREA);
    }

    reti = mapperSUT->SyncStore();
    EXPECT_EQ(reti, RET_INT_SUCCESS);
    _UnmountVolume(0);
}

TEST_F(MapperTest, LoadAndGet)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.LoadAndGet ********************");
    _LoadVolume(0);
    _MountVolume(0);
    for (int i = 0; i < NUM_MAP_ENTRY; ++i)
    {
        _CheckStripeMap(vsids0[i], lsids0[i], IN_WRITE_BUFFER_AREA);
        _CheckVSAMap(0, rbas0[i], vsids0[i], offsets0[i]);
    }
    _UnmountVolume(0);
    _DeleteVolume(0);
    _DeleteStripeMapFile();

    // Pre-setting for next test
    _CreateVolume(0);
    _CreateVolume(1);
}

TEST_F(MapperTest, EmptyVSAMapInternalLoading)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.EmptyVSAMapInternalLoading ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    _LoadVolume(1);
    VirtualBlkAddr vsa;
    int caller;

    do
    {
        usleep(1);
        IBOF_TRACE_INFO(9999, "Call GetVSAInternal()");
        caller = CALLER_EVENT;
        vsa = mapperSUT->GetVSAInternal(0, 0, caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    EXPECT_EQ(vsa, UNMAP_VSA);

    do
    {
        usleep(1);
        IBOF_TRACE_INFO(9999, "Call GetVSAInternal()");
        caller = CALLER_NOT_EVENT;
        vsa = mapperSUT->GetVSAInternal(1, 0, caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    EXPECT_EQ(vsa, UNMAP_VSA);

    // Pre-setting for next test
    _MountVolume(0);
    _MountVolume(1);

    FillRandomSection(0, _GetRbaMax(0), rbas0, NUM_MAP_ENTRY);
    FillRandomSection(1, _GetRbaMax(1), rbas1, NUM_MAP_ENTRY);
    FillRandomSection(0, TEST_VSID_MAX, vsids0, NUM_MAP_ENTRY);
    FillRandomSection(1, TEST_VSID_MAX, vsids1, NUM_MAP_ENTRY);
    FillRandomSection(0, TEST_OFFSET_MAX, offsets0, NUM_MAP_ENTRY);
    FillRandomSection(1, TEST_OFFSET_MAX, offsets1, NUM_MAP_ENTRY);
    for (int i = 0; i < NUM_MAP_ENTRY; ++i)
    {
        _UpdateVSAMap(0, rbas0[i], vsids0[i], offsets0[i]);
        _UpdateVSAMap(1, rbas1[i], vsids1[i], offsets1[i]);
    }

    _UnmountVolume(0);
    _UnmountVolume(1);
    _DeleteMockVolumeManager();
}

TEST_F(MapperTest, RandomWrittenVSAMapInternalLoading)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.RandomWrittenVSAMapInternalLoading ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    _LoadVolume(1);
    VirtualBlkAddr vsa;
    int caller;

    do
    {
        usleep(1);
        IBOF_TRACE_INFO(9999, "Call GetVSAInternal()");
        caller = CALLER_EVENT;
        vsa = mapperSUT->GetVSAInternal(0, rbas0[0], caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    VirtualBlkAddr vsaOrig = {.stripeId = vsids0[0], .offset = offsets0[0]};
    EXPECT_EQ(vsa, vsaOrig);

    do
    {
        usleep(1);
        IBOF_TRACE_INFO(9999, "Call GetVSAInternal()");
        caller = CALLER_NOT_EVENT;
        vsa = mapperSUT->GetVSAInternal(1, rbas1[0], caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    vsaOrig = {.stripeId = vsids1[0], .offset = offsets1[0]};
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
    _UnmountVolume(1);
    _DeleteMockVolumeManager();
}

TEST_F(MapperTest, InternalLoadingAndFGMount)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.InternalLoadingAndFGMount ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    _LoadVolume(1);

    VirtualBlkAddr vsa;
    int caller;

    caller = CALLER_EVENT;
    vsa = mapperSUT->GetVSAInternal(0, rbas0[0], caller);

    _MountVolume(0);
    // Pre-setting for next test
    _UnmountVolume(0);
    _DeleteVolume(0);
    _DeleteVolume(1);
    _DeleteMockVolumeManager();
}

TEST_F(MapperTest, StripeMapHeaderFlushAndLoad)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.StripeMapHeaderFlushAndLoad ********************");
    int reti = 0;
    _CreateVolume(0);
    _MountVolume(0);

    StripeMapContent* stripemap = _GetStripeMap();
    MapHeader* header = _GetMapHeader(stripemap);
    MapIoHandler* flushHandler = _GetFlushHandler(stripemap);

    // Original MapHeader
    char* buf1 = (char*)malloc(header->size);
    header->CopyToBuffer(buf1);
    MapHeader* orgHeader = (MapHeader*)buf1;

    // Flush Original MapHeader
    reti = flushHandler->SaveHeader(); // Sync IO
    EXPECT_EQ(reti, RET_INT_SUCCESS);

    // Clear Original MapHeader.MpageValidInfo
    header->SetMpageValidInfo(0, 0);

    // Load MapHeader
    reti = flushHandler->LoadFromMFS(); // Sync IO
    EXPECT_EQ(reti, RET_INT_SUCCESS);

    char* buf2 = (char*)malloc(header->size);
    header->CopyToBuffer(buf2);
    MapHeader* loadedHeader = (MapHeader*)buf2;

    EXPECT_EQ(orgHeader->mpageData.numValidMpages,
        loadedHeader->mpageData.numValidMpages);
    EXPECT_EQ(orgHeader->mpageData.numTotalMpages,
        loadedHeader->mpageData.numTotalMpages);

    free(buf1);
    free(buf2);

    _UnmountVolume(0);
    _DeleteVolume(0);
}

TEST_F(MapperTest, VSAMapHeaderFlushAndLoad)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.VSAMapHeaderFlushAndLoad ********************");
    int reti = 0;
    _CreateVolume(0);
    _MountVolume(0);

    VSAMapContent* vsamap = _GetVSAMap(0);
    MapHeader* header = _GetMapHeader(vsamap);
    MapIoHandler* flushHandler = _GetFlushHandler(vsamap);

    // Original MapHeader
    char* buf1 = (char*)malloc(header->size);
    header->CopyToBuffer(buf1);
    MapHeader* orgHeader = (MapHeader*)buf1;

    // Flush Original MapHeader
    reti = flushHandler->SaveHeader(); // Sync IO
    EXPECT_EQ(reti, RET_INT_SUCCESS);

    // Clear Original MapHeader.MpageValidInfo
    header->SetMpageValidInfo(0, 0);

    // Load MapHeader
    reti = flushHandler->LoadFromMFS(); // Sync IO
    EXPECT_EQ(reti, RET_INT_SUCCESS);

    char* buf2 = (char*)malloc(header->size);
    header->CopyToBuffer(buf2);
    MapHeader* loadedHeader = (MapHeader*)buf2;

    EXPECT_EQ(orgHeader->mpageData.numValidMpages,
        loadedHeader->mpageData.numValidMpages);
    EXPECT_EQ(orgHeader->mpageData.numTotalMpages,
        loadedHeader->mpageData.numTotalMpages);

    free(buf1);
    free(buf2);

    _UnmountVolume(0);
    _DeleteVolume(0);
}

TEST_F(MapperTest, LinkReverseMap)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.LinkReverseMap ********************");
    int reti = 0;
    Stripe stripe;

    reti = mapperSUT->LinkReverseMap(&stripe, 0, 0);
    EXPECT_EQ(reti, RET_INT_SUCCESS);
}

TEST_F(MapperTest, CheckLoadAndGetHeader)
{
    IBOF_TRACE_INFO(9999, "******************** MapperTest.CheckLoadAndGetHeader ********************");
    bool retb = true;
    int reti = 0;

    retb = _IsVSAMapLoaded(TEST_VOL_ID);
    EXPECT_NE(retb, RET_BOOL_SUCCESS);

    _CreateVolume(0);
    retb = _IsVSAMapLoaded(TEST_VOL_ID);
    EXPECT_EQ(retb, RET_BOOL_SUCCESS);

    _MountVolume(0);
    _UnmountVolume(0);
    uint64_t volSizeByte = 0;
    reti = _GetVolSizeByLoadHeader(TEST_VOL_ID, volSizeByte);
    EXPECT_EQ(reti, RET_INT_SUCCESS);

    uint64_t totalBlksLoaded = DivideUp(volSizeByte, PAGE_SIZE);
    uint64_t totalBlks = DivideUp(createdVolumeInfo[0], PAGE_SIZE);
    EXPECT_EQ(totalBlksLoaded, totalBlks);
}

//------------------------------------------------------------------------------

MapFlushDoneEvent::MapFlushDoneEvent(int mapId, MapFlushHandlerTest* tester)
: mapId(mapId),
  tester(tester)
{
}

MapFlushDoneEvent::~MapFlushDoneEvent(void)
{
}

bool
MapFlushDoneEvent::Execute(void)
{
    tester->FlushDoneCallback(mapId);
    return true;
}

void
MapFlushHandlerTest::SetUp(void)
{
    MapperTest::SetUp();
    _CreateVolume(0);
    _MountVolume(0);

    _SetupMockEventScheduler();
}

void
MapFlushHandlerTest::TearDown(void)
{
    _UnmountVolume(0);
    _DeleteVolume(0);
    MapperTest::TearDown();

    _ResetMockEventScheduler();
}

void
MapFlushHandlerTest::_SimulateNPOR(void)
{
    _UnmountVolume(0);
    MapperSingleton::ResetInstance();

    mapperSUT = MapperSingleton::Instance();
    mapperSUT->Init();
    _LoadVolume(0);
    _MountVolume(0);
}

void
MapFlushHandlerTest::_SimulateSPOR(void)
{
    MapperSingleton::ResetInstance();

    mapperSUT = MapperSingleton::Instance();
    mapperSUT->Init();
    _LoadVolume(0);
    _MountVolume(0);
}

void
MapFlushHandlerTest::_SetupMockEventScheduler(void)
{
    cpu_set_t zeroed;
    CPU_ZERO(&zeroed);

    EventArgument::SetStaticMember(new EventScheduler(1, zeroed, zeroed), nullptr);
}

void
MapFlushHandlerTest::_ResetMockEventScheduler(void)
{
    delete EventArgument::GetEventScheduler();
}

void
MapFlushHandlerTest::FlushDoneCallback(int mapId)
{
    numMapsFlushed++;
    if (numMapsFlushed == numMapsToFlush)
    {
        flushCompleted = true;
    }
}

void
MapFlushHandlerTest::_VSAMapDirtyUpdateTester(int volId, BlkAddr rba,
    StripeId vsidOrig, BlkOffset offsetOrig, MpageList& dirtyPages)
{
    _UpdateVSAMap(volId, rba, vsidOrig, offsetOrig);
    _AddToDirtyPages(dirtyPages, (_GetVSAMap(volId))->GetDirtyPages(rba, 1));
}

void
MapFlushHandlerTest::_StripeMapDirtyUpdateTester(StripeId vsid, StripeId lsid,
    StripeLoc loc, MpageList& dirtyPages)
{
    _UpdateStripeMap(vsid, lsid, loc);
    _AddToDirtyPages(dirtyPages, (_GetStripeMap())->GetDirtyPages(vsid, 1));
}

void
MapFlushHandlerTest::_AddToDirtyPages(MpageList& dirtyPages, MpageList pageList)
{
    dirtyPages.insert(pageList.begin(), pageList.end());
}

void
MapFlushHandlerTest::_TriggerDirtyFlush(int mapId, MpageList& dirtyPages)
{
    EventSmartPtr callback(new MapFlushDoneEvent(mapId, this));
    mapperSUT->StartDirtyPageFlush(mapId, dirtyPages, callback);
}

void
MapFlushHandlerTest::_TriggerFlush(int mapId)
{
    EventSmartPtr callback(new MapFlushDoneEvent(mapId, this));
    mapperSUT->FlushMap(mapId, callback);
}

// [VSAMap Validity /w POR]
// Store VSA Map -> Unmount (NPO Case) -> Load VSA Map
TEST_F(MapFlushHandlerTest, StoreVSAMap)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.StoreVSAMap ********************");
    uint64_t test_count = SZ_1GB / ibofos::BLOCK_SIZE;
    uint64_t test;

    for (test = 0; test < test_count; test++)
    {
        BlkAddr rba = test;
        _UpdateVSAMap(TEST_VOL_ID, rba, (StripeId)(test / 128), (BlkOffset)(test % 128));
    }

    mapperSUT->SyncStore(); // store stripMap & vsamap[vol]

    _SimulateSPOR();

    for (test = 0; test < test_count; test++)
    {
        BlkAddr rba = test;
        _CheckVSAMap(TEST_VOL_ID, rba, (StripeId)(test / 128), (BlkOffset)(test % 128));
    }
}

// [StripMap Validity /w POR]
// Store Stripe Map -> Unmount (NPO Case) -> Load Stripe Map
TEST_F(MapFlushHandlerTest, StoreStripeMap)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.StoreStripeMap ********************");
    int test;
    for (test = 0; test < 100; test++)
    {
        StripeLoc loc = (test % 2 ? IN_USER_AREA : IN_WRITE_BUFFER_AREA);
        _UpdateStripeMap(test, test * 2, loc);
    }

    mapperSUT->SyncStore();

    _SimulateSPOR();

    for (test = 0; test < 100; test++)
    {
        StripeLoc loc = (test % 2 ? IN_USER_AREA : IN_WRITE_BUFFER_AREA);
        _CheckStripeMap((StripeId)test, test * 2, (StripeLoc)loc);
    }
}

TEST_F(MapFlushHandlerTest, FlushSingleDirtyPage)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.FlushSingleDirtyPage ********************");
    MpageList dirtyPages;
    numMapsToFlush = 1;
    numMapsFlushed = 0;
    flushCompleted = false;

    BlkAddr rba = 0;
    _VSAMapDirtyUpdateTester(TEST_VOL_ID, rba, 3, 3, dirtyPages);

    _TriggerDirtyFlush(TEST_VOL_ID, dirtyPages);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _CheckVSAMap(TEST_VOL_ID, rba, 3, 3);

    _SimulateSPOR();

    _CheckVSAMap(TEST_VOL_ID, rba, 3, 3);
}

TEST_F(MapFlushHandlerTest, FlushDirtyPages)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.FlushDirtyPages ********************");
    MpageList dirtyPages;
    numMapsToFlush = 1;
    numMapsFlushed = 0;
    flushCompleted = false;

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        BlkAddr rba = (BlkAddr)testCnt;
        _VSAMapDirtyUpdateTester(TEST_VOL_ID, rba, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128, dirtyPages);
    }

    _TriggerDirtyFlush(TEST_VOL_ID, dirtyPages);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        _CheckVSAMap(TEST_VOL_ID, (BlkAddr)testCnt, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128);
    }
}

TEST_F(MapFlushHandlerTest, FlushDirtyMaps)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.FlushDirtyMaps ********************");
    MpageList vsaMapDirtyPages;
    MpageList stripeMapDirtyPages;

    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        BlkAddr rba = (BlkAddr)testCnt;
        _VSAMapDirtyUpdateTester(TEST_VOL_ID, rba, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128, vsaMapDirtyPages);
    }

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _StripeMapDirtyUpdateTester(vsid, lsid, IN_WRITE_BUFFER_AREA,
            stripeMapDirtyPages);
    }

    _TriggerDirtyFlush(TEST_VOL_ID, vsaMapDirtyPages);
    _TriggerDirtyFlush(STRIPE_MAP_ID, stripeMapDirtyPages);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        _CheckVSAMap(TEST_VOL_ID, (BlkAddr)testCnt, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _CheckStripeMap(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }
}

TEST_F(MapFlushHandlerTest, AsyncFlushMap)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.AsyncFlushMap ********************");
    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        BlkAddr rba = (BlkAddr)testCnt;
        _UpdateVSAMap(TEST_VOL_ID, rba, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < 1000; ++testCnt)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _UpdateStripeMap(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }

    _TriggerFlush(TEST_VOL_ID);
    _TriggerFlush(STRIPE_MAP_ID);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        _CheckVSAMap(TEST_VOL_ID, (BlkAddr)testCnt, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < 1000; testCnt++)
    {
        StripeId vsid = (StripeId)testCnt;
        StripeId lsid = (StripeId)testCnt % 128;
        _CheckStripeMap(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }
}

TEST_F(MapFlushHandlerTest, AsyncFlushEmptyMap)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.AsyncFlushEmptyMap ********************");
    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    _TriggerFlush(TEST_VOL_ID);
    _TriggerFlush(STRIPE_MAP_ID);
    while (flushCompleted != true)
    {
        usleep(1);
    }
}

TEST_F(MapFlushHandlerTest, AsnyFlushMapSeveralMpages)
{
    IBOF_TRACE_INFO(9999, "******************** MapFlushHandlerTest.AsnyFlushMapSeveralMpages ********************");
    numMapsToFlush = 2;
    numMapsFlushed = 0;
    flushCompleted = false;

    int numMpagesToTest = 500;

    for (int testCnt = 0; testCnt < numMpagesToTest; ++testCnt)
    {
        BlkAddr rba = (BlkAddr)(testCnt * (PAGE_SIZE / 8));
        _UpdateVSAMap(TEST_VOL_ID, rba, (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < numMpagesToTest; ++testCnt)
    {
        StripeId vsid = (StripeId)(testCnt * (PAGE_SIZE / 4));
        StripeId lsid = (StripeId)testCnt % 128;
        _UpdateStripeMap(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }

    _TriggerFlush(TEST_VOL_ID);
    _TriggerFlush(STRIPE_MAP_ID);
    while (flushCompleted != true)
    {
        usleep(1);
    }

    _SimulateSPOR();

    for (int testCnt = 0; testCnt < numMpagesToTest; testCnt++)
    {
        _CheckVSAMap(TEST_VOL_ID, (BlkAddr)(testCnt * (PAGE_SIZE / 8)), (StripeId)testCnt / 128,
            (BlkOffset)testCnt % 128);
    }

    for (int testCnt = 0; testCnt < numMpagesToTest; testCnt++)
    {
        StripeId vsid = (StripeId)(testCnt * (PAGE_SIZE / 4));
        StripeId lsid = (StripeId)testCnt % 128;
        _CheckStripeMap(vsid, lsid, IN_WRITE_BUFFER_AREA);
    }
}

} // namespace ibofos
