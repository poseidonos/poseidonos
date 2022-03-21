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

#include "test/integration-tests/mapper/utils/mapper_it_const.h"
#include "test/integration-tests/mapper/mapper_it_test.h"

namespace pos
{
void
MapperTest::SetUp(void)
{
    MapperTestFixture::SetUp();
}

void
MapperTest::TearDown(void)
{
    MapperTestFixture::TearDown();
}

void
AccessRequest::CLIThreadFunc(VSAMapManager* iVSAMap, BlkAddr rba)
{
    POS_TRACE_INFO(9999, "Call GetVSAInternal(), CLI");
    VirtualBlkAddr vsa;
    vsa = iVSAMap->GetVSAWoCond(0, 0);
    POS_TRACE_INFO(9999, "CLI Done");
}

//------------------------------------------------------------------------------
TEST_F(MapperTestFixture, SetAndStore)
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

    reti = mapperSUT->StoreAll();
    EXPECT_EQ(reti, RET_INT_SUCCESS);
    _UnmountVolume(0);
}

TEST_F(MapperTestFixture, LoadAndGet)
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

TEST_F(MapperTestFixture, EmptyVSAMapInternalLoading)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.EmptyVSAMapInternalLoading ********************");

    _LoadVolume(0);
    _LoadVolume(1);
    VirtualBlkAddr vsa;
    int retry = false;
    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), event");
        vsa = mapperSUT->GetVSAInternal(0, 0, retry);
    } while (UNMAP_VSA == vsa);
    EXPECT_EQ(vsa, UNMAP_VSA);

    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), not event");
        vsa = mapperSUT->GetVSAInternal(1, 0, retry);
    } while (UNMAP_VSA == vsa);
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
}

TEST_F(MapperTestFixture, RandomWrittenVSAMapInternalLoading_1)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_1 ********************");
    _LoadVolume(0);
    _LoadVolume(1);
    VirtualBlkAddr vsa;
    int retry = false;
    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), event");
        vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);
    } while (UNMAP_VSA == vsa);
    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    do
    {
        usleep(1);
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), not event");
        vsa = mapperSUT->GetVSAInternal(0, rbas1[1], retry);
    } while (UNMAP_VSA == vsa);
    vsaOrig = { .stripeId = vsids1[0], .offset = offsets1[0] };
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
    _UnmountVolume(1);
}

TEST_F(MapperTestFixture, RandomWrittenVSAMapInternalLoading_2)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_2 ********************");
    _LoadVolume(0);
    VirtualBlkAddr vsa;
    POS_TRACE_INFO(9999, "Call GetVSAInternal(), event_1");
    int retry = false;
    vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);

    do
    {
        POS_TRACE_INFO(9999, "Call GetVSAInternal(), event_2");
        vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);
    } while (UNMAP_VSA == vsa);
    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
}

TEST_F(MapperTestFixture, RandomWrittenVSAMapInternalLoading_3)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_3 ********************");
    _LoadVolume(0);
    VirtualBlkAddr vsa;

    POS_TRACE_INFO(9999, "Call GetVSAInternal(), event");
    int retry = false;
    vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);

    POS_TRACE_INFO(9999, "Call GetVSAInternal(), NOT event");
    vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);

    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    // Pre-setting for next test
    _UnmountVolume(0);
}

TEST_F(MapperTestFixture, RandomWrittenVSAMapInternalLoading_4)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.RandomWrittenVSAMapInternalLoading_4 ********************");
    _LoadVolume(0);
    VirtualBlkAddr vsa;

    AccessRequest req;
    std::thread t{&AccessRequest::CLIThreadFunc, &req, vsaMapManagerSUT, rbas0[0]};

    usleep(200);
    POS_TRACE_INFO(9999, "Call GetVSAInternal(), NOT event");
    int retry = false;
    vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);
    POS_TRACE_INFO(9999, "Let's check");
    VirtualBlkAddr vsaOrig = { .stripeId = vsids0[0], .offset = offsets0[0] };
    EXPECT_EQ(vsa, vsaOrig);

    t.join();
    // Pre-setting for next test
    _UnmountVolume(0);
}

TEST_F(MapperTestFixture, InternalLoadingAndFGMount)
{
    POS_TRACE_INFO(9999, "******************** VSAMapManagerTest.InternalLoadingAndFGMount ********************");
    _LoadVolume(0);
    _LoadVolume(1);

    VirtualBlkAddr vsa;
    int retry = false;
    vsa = mapperSUT->GetVSAInternal(0, rbas0[0], retry);
    _MountVolume(0);

    // Pre-setting for next test
    _UnmountVolume(0);
    _DeleteVolume(0);
    _DeleteVolume(1);
}

}   // namespace pos
