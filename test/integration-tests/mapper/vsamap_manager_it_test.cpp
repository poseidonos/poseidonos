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

#include "test/integration-tests/mapper/utils/mapper_it_const.h"
#include "test/integration-tests/mapper/vsamap_manager_it_test.h"

namespace pos
{

void
VSAMapManagerTest::SetUp(void)
{
    MapperTestFixture::SetUp();
}

void
VSAMapManagerTest::TearDown(void)
{
    MapperTestFixture::TearDown();
}

void
AccessRequest::CLIThreadFunc(IVSAMap* iVSAMap, BlkAddr rba)
{
    POS_TRACE_INFO(9999, "Call GetVSAInternal(), CLI");
    int caller = CALLER_NOT_EVENT;
    iVSAMap->GetVSAInternal(0, rba, caller);
    POS_TRACE_INFO(9999, "CLI Done");
}

//------------------------------------------------------------------------------

TEST_F(VSAMapManagerTest, SetAndStore)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.SetAndStore ********************");
    int reti = 0;
    _CreateRandVolume(0);
    _MountVolume(0);

    _FillRandomSection(0, _GetRbaMax(0), rbas0, NUM_MAP_ENTRY);
    _PrintData("RBA   ", rbas0);
    _FillRandomSection(0, TEST_VSID_MAX, vsids0, NUM_MAP_ENTRY);
    _PrintData("VSID  ", vsids0);
    _FillRandomSection(0, TEST_OFFSET_MAX, offsets0, NUM_MAP_ENTRY);
    _PrintData("OFFSET", offsets0);
    for (int i = 0; i < NUM_MAP_ENTRY; ++i)
    {
        _SetVSAs(0, rbas0[i], vsids0[i], offsets0[i]);
    }

    reti = vsaMapManagerSUT->StoreMaps();
    EXPECT_EQ(reti, RET_INT_SUCCESS);
    _UnmountVolume(0);
}

TEST_F(VSAMapManagerTest, LoadAndGet)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.LoadAndGet ********************");
    _LoadVolume(0);
    _MountVolume(0);
    for (int i = 0; i < NUM_MAP_ENTRY; ++i)
    {
        _GetAndCompareVSA(0, rbas0[i], vsids0[i], offsets0[i]);
    }
    _UnmountVolume(0);
    _DeleteVolume(0);

    // Pre-setting for next test
    _CreateRandVolume(0);
    _CreateRandVolume(1);
}

TEST_F(VSAMapManagerTest, EmptyVSAMapInternalLoading)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.EmptyVSAMapInternalLoading ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    _LoadVolume(1);
    VirtualBlkAddr vsa;
    int caller;
    IVSAMap* iVSAMap = vsaMapManagerSUT->GetIVSAMap();

    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), event");
        caller = CALLER_EVENT;
        vsa = iVSAMap->GetVSAInternal(0, 0, caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    EXPECT_EQ(vsa, UNMAP_VSA);

    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), not event");
        caller = CALLER_NOT_EVENT;
        vsa = iVSAMap->GetVSAInternal(1, 0, caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    EXPECT_EQ(vsa, UNMAP_VSA);

    // Pre-setting for next test
    _MountVolume(0);
    _MountVolume(1);

    _FillRandomSection(0, _GetRbaMax(0), rbas0, NUM_MAP_ENTRY);
    _FillRandomSection(1, _GetRbaMax(1), rbas1, NUM_MAP_ENTRY);
    _FillRandomSection(0, TEST_VSID_MAX, vsids0, NUM_MAP_ENTRY);
    _FillRandomSection(1, TEST_VSID_MAX, vsids1, NUM_MAP_ENTRY);
    _FillRandomSection(0, TEST_OFFSET_MAX, offsets0, NUM_MAP_ENTRY);
    _FillRandomSection(1, TEST_OFFSET_MAX, offsets1, NUM_MAP_ENTRY);
    for (int i = 0; i < NUM_MAP_ENTRY; ++i)
    {
        _SetVSAs(0, rbas0[i], vsids0[i], offsets0[i]);
        _SetVSAs(1, rbas1[i], vsids1[i], offsets1[i]);
    }

    _UnmountVolume(0);
    _UnmountVolume(1);
    _DeleteMockVolumeManager();
}

TEST_F(VSAMapManagerTest, RandomWrittenVSAMapInternalLoading_1)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_1 ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    _LoadVolume(1);
    VirtualBlkAddr vsa;
    int caller;
    IVSAMap* iVSAMap = vsaMapManagerSUT->GetIVSAMap();

    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), event");
        caller = CALLER_EVENT;
        vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), not event");
        caller = CALLER_NOT_EVENT;
        vsa = iVSAMap->GetVSAInternal(1, rbas1[0], caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    vsaOrig = { .stripeId = vsids1[0], .offset = offsets1[0] };
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
    _UnmountVolume(1);
    _DeleteMockVolumeManager();
}

TEST_F(VSAMapManagerTest, RandomWrittenVSAMapInternalLoading_2)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_2 ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    VirtualBlkAddr vsa;
    int caller;
    IVSAMap* iVSAMap = vsaMapManagerSUT->GetIVSAMap();

    POS_TRACE_INFO(9999, "Call GetVSAInternal(), event_1");
    caller = CALLER_EVENT;
    vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);

    do
    {
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), event_2");
        caller = CALLER_EVENT;
        vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);
    } while (UNMAP_VSA == vsa && NEED_RETRY == caller);
    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
    _DeleteMockVolumeManager();
}

TEST_F(VSAMapManagerTest, RandomWrittenVSAMapInternalLoading_3)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_3 ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    VirtualBlkAddr vsa;
    int caller;
    IVSAMap* iVSAMap = vsaMapManagerSUT->GetIVSAMap();

    POS_TRACE_INFO(9999, "Call GetVSAInternal(), event");
    caller = CALLER_EVENT;
    vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);

    POS_TRACE_INFO(9999, "Call GetVSAInternal(), NOT event");
    caller = CALLER_NOT_EVENT;
    vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);

    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
    _DeleteMockVolumeManager();
}

TEST_F(VSAMapManagerTest, RandomWrittenVSAMapInternalLoading_4)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_4 ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    VirtualBlkAddr vsa;
    int caller;
    IVSAMap* iVSAMap = vsaMapManagerSUT->GetIVSAMap();

    AccessRequest req;
    std::thread t{&AccessRequest::CLIThreadFunc, &req, iVSAMap, rbas0[0]};

    usleep(200);
    POS_TRACE_INFO(9999, "Call GetVSAInternal(), NOT event");
    caller = CALLER_NOT_EVENT;
    vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);
    POS_TRACE_INFO(9999, "Let's check");
    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    t.join();
    // Pre-setting for next test
    _UnmountVolume(0);
    _DeleteMockVolumeManager();
}

TEST_F(VSAMapManagerTest, InternalLoadingAndFGMount)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.InternalLoadingAndFGMount ********************");
    _UseMockVolumeManager();
    _LoadVolume(0);
    _LoadVolume(1);

    VirtualBlkAddr vsa;
    int caller;
    IVSAMap* iVSAMap = vsaMapManagerSUT->GetIVSAMap();

    caller = CALLER_EVENT;
    vsa = iVSAMap->GetVSAInternal(0, rbas0[0], caller);
    _MountVolume(0);

    // Pre-setting for next test
    _UnmountVolume(0);
    _DeleteVolume(0);
    _DeleteVolume(1);
    _DeleteMockVolumeManager();
}

}   // namespace pos
