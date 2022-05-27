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

#include "test/integration-tests/mapper/fixtures/mapper_test_fixture.h"

#include <string>

#include "src/include/smart_ptr_type.h"
#include "src/metafs/include/metafs_service.h"
#include "test/integration-tests/mapper/test_doubles/map_flush_done_event_mock.h"
#include "test/integration-tests/mapper/utils/mapper_it_const.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"

namespace pos
{
std::map<int, uint64_t> MapperTestFixture::createdVolumeInfo;
std::vector<uint32_t> MapperTestFixture::rbas0, MapperTestFixture::vsids0, MapperTestFixture::offsets0;
std::vector<uint32_t> MapperTestFixture::rbas1, MapperTestFixture::vsids1, MapperTestFixture::offsets1;

void
MapperTestFixture::SetUp(void)
{
    mockArrayInfo = new MockArrayInfo;
    StateControl is("array");
    addrInfo = new MapperAddressInfo(mockArrayInfo);
    volManager = new VolumeManager(mockArrayInfo, &is);
    vsaMapManagerSUT = new VSAMapManager(nullptr, nullptr, addrInfo);
    stripeMapManagerSUT = new StripeMapManager(nullptr, addrInfo);
    reverseMapManagerSUT = new ReverseMapManager(vsaMapManagerSUT, stripeMapManagerSUT, volManager, addrInfo);
    addrInfo->SetIsUT(true);
    mapperSUT = new Mapper(nullptr, vsaMapManagerSUT, stripeMapManagerSUT, reverseMapManagerSUT, addrInfo, mockArrayInfo, nullptr);
    mapperSUT->Init();
}

void
MapperTestFixture::TearDown(void)
{
    mapperSUT->Dispose();
    delete mapperSUT;
    delete mockArrayInfo;
}

int
MapperTestFixture::GetSavedVolumeSize(int volId, uint64_t& volSize)
{
    volSize = createdVolumeInfo[volId];
    return 0;
}

void
MapperTestFixture::_CreateRandVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    uint64_t volSizeByte = _GetRandomSection(10, 20) * SZ_1M;
    POS_TRACE_WARN(9999, "Volume Name:{}  Size(Byte):{}", volName, volSizeByte);

    bool reb = mapperSUT->VolumeCreated(volId, volSizeByte);
    EXPECT_EQ(reb, true);
    createdVolumeInfo.emplace(volId, volSizeByte);
}

void
MapperTestFixture::_LoadVolume(int volId)
{
    uint64_t volSizeByte = _GetRandomSection(10, 20) * SZ_1M;
    uint64_t volSizeByte = 5 * SZ_1M;
    bool retb = mapperSUT->VolumeLoaded(volId, volSizeByte);
    EXPECT_EQ(retb, true);
}

void
MapperTestFixture::_MountVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    uint64_t volSizeByte = _GetRandomSection(10, 20) * SZ_1M;
    bool retb = mapperSUT->VolumeMounted(volId, volSizeByte);
    EXPECT_EQ(retb, true);
}

void
MapperTestFixture::_UnmountVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    bool retb = mapperSUT->VolumeUnmounted(volId, false);
    EXPECT_EQ(retb, true);
}

void
MapperTestFixture::_DeleteVolume(int volId)
{
    std::string volName = "Vol" + std::to_string(volId);
    int retb = mapperSUT->PrepareVolumeDelete(volId);
    EXPECT_EQ(retb, 0);
    retb = mapperSUT->DeleteVolumeMap(volId);
    EXPECT_EQ(retb, 0);
    createdVolumeInfo.erase(volId);
}

BlkAddr
MapperTestFixture::_GetRbaMax(int volId)
{
    return (createdVolumeInfo[volId] / PAGE_SIZE_IT) - 1;
}

void
MapperTestFixture::_SetVSAs(int volumeId, BlkAddr rba, StripeId vsidOrig, BlkOffset offsetOrig)
{
    VirtualBlkAddr vsaOrig = {
        .stripeId = vsidOrig,
        .offset = offsetOrig};
    VirtualBlks vb = {
        .startVsa = vsaOrig,
        .numBlks = 1};
    IVSAMap* iVSAMap = mapperSUT->GetIVSAMap();
    int reti = iVSAMap->SetVSAs(volumeId, rba, vb);
    EXPECT_EQ(reti, RET_INT_SUCCESS);
}

void
MapperTestFixture::_GetAndCompareVSA(int volumeId, BlkAddr rba, StripeId vsidOrig, BlkOffset offsetOrig)
{
    VsaArray va;
    IVSAMap* iVSAMap = mapperSUT->GetIVSAMap();
    int reti = iVSAMap->GetVSAs(volumeId, rba, 1, va);
    EXPECT_EQ(reti, RET_INT_SUCCESS);

    VirtualBlkAddr vsaOrig = {
        .stripeId = vsidOrig,
        .offset = offsetOrig};
    EXPECT_EQ(va[0], vsaOrig);
}

void
MapperTestFixture::_SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    IStripeMap* iStripeMap = mapperSUT->GetIStripeMap();
    int reti = iStripeMap->SetLSA(vsid, lsid, loc);
    EXPECT_EQ(reti, RET_INT_SUCCESS);
}

void
MapperTestFixture::_GetAndCompareLSA(StripeId vsid, StripeId lsidOrig, StripeLoc locOrig)
{
    StripeAddr lsaOrig = {
        .stripeLoc = locOrig,
        .stripeId = lsidOrig};
    IStripeMap* iStripeMap = mapperSUT->GetIStripeMap();
    StripeAddr lsa = iStripeMap->GetLSA(vsid);
    EXPECT_EQ(lsa, lsaOrig);
}

void
MapperTestFixture::_SimulateNPOR(void)
{
    _UnmountVolume(0);
    delete mapperSUT;

    mapperSUT = new Mapper(nullptr, mockArrayInfo, nullptr);
    mapperSUT->Init();
    _LoadVolume(0);
    _MountVolume(0);
}

void
MapperTestFixture::_SimulateSPOR(void)
{
    delete mapperSUT;

    mapperSUT = new Mapper(nullptr, mockArrayInfo, nullptr);
    mapperSUT->Init();
    _LoadVolume(0);
    _MountVolume(0);
}

void
MapperTestFixture::_VSAMapDirtyUpdateTester(int volId, BlkAddr rba, StripeId vsidOrig, BlkOffset offsetOrig, MpageList& dirtyPages)
{
    _SetVSAs(volId, rba, vsidOrig, offsetOrig);
    _AddToDirtyPages(dirtyPages, vsaMapManagerSUT->GetVSAMapContent(volId)->GetDirtyPages(rba, 1));
}

void
MapperTestFixture::_StripeMapDirtyUpdateTester(StripeId vsid, StripeId lsid, StripeLoc loc, MpageList& dirtyPages)
{
    _SetLSA(vsid, lsid, loc);
    _AddToDirtyPages(dirtyPages, stripeMapManagerSUT->GetStripeMapContent()->GetDirtyPages(vsid, 1));
}

void
MapperTestFixture::_AddToDirtyPages(MpageList& dirtyPages, MpageList pageList)
{
    dirtyPages.insert(pageList.begin(), pageList.end());
}

void
MapperTestFixture::_FlushDirtyPagesGiven(int mapId, MpageList& dirtyPages)
{
    EventSmartPtr callback(new MapFlushDoneEvent(mapId, this));
    mapperSUT->FlushDirtyMpages(mapId, callback);
}

void
MapperTestFixture::_FlushTouchedPages(int mapId)
{
    EventSmartPtr callback(new MapFlushDoneEvent(mapId, this));
    mapperSUT->FlushDirtyMpages(mapId, callback);
}

void
MapperTestFixture::FlushDoneCallback(int mapId)
{
    numMapsFlushed++;
    if (numMapsFlushed == numMapsToFlush)
    {
        flushCompleted = true;
    }
}

} // namespace pos
