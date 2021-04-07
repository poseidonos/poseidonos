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

#include "vsamap_manager_test.h"

namespace ibofos
{
std::map<int, uint64_t> VSAMapManagerTest::volumeInfo;

void
VSAMapManagerTest::SetUp(void)
{
    vsaMapManagerSUT = new VSAMapManager();
}

void
VSAMapManagerTest::TearDown(void)
{
    delete vsaMapManagerSUT;
}

void
VSAMapManagerTest::_CreateRandVolume(int volId)
{
    // int volId = numVolumeCreated;
    std::string volName = "Vol" + std::to_string(volId);
    uint64_t volSizeByte = _GetRandomSection(1, 50) * SZ_1G; // 1 ... 50 GB

    std::cout << "Volume Name: " << volName << "  Size(Byte): " << volSizeByte << std::endl;
    bool reb = vsaMapManagerSUT->VolumeCreated(volName, volId, volSizeByte, 0, 0);
    EXPECT_EQ(reb, true);
    volumeInfo.emplace(volId, volSizeByte);
}

void
VSAMapManagerTest::_LoadVolume(int volId)
{
    bool retb = vsaMapManagerSUT->VolumeLoaded("", volId, 0, 0, 0);
    EXPECT_EQ(retb, true);
}

int
VSAMapManagerTest::GetSavedVolumeSize(int volId, uint64_t& volSize)
{
    volSize = volumeInfo[volId];
    return 0;
}

// Create 3 random size volumes
TEST_F(VSAMapManagerTest, Create3RandomVolume)
{
    _CreateRandVolume(0);
    _CreateRandVolume(1);
    _CreateRandVolume(2);
}

// Empty VSAMap Internal loading
TEST_F(VSAMapManagerTest, EmptyVSAMapInternalLoading)
{
    MockVolumeManager* mockVolumeManager = new MockVolumeManager();
    EXPECT_CALL(*mockVolumeManager, GetVolumeSize(_, _)).WillRepeatedly(Invoke(this, &VSAMapManagerTest::GetSavedVolumeSize));
    vsaMapManagerSUT->SetVolumeManagerObject(mockVolumeManager);

    LoadVolume(0);
    LoadVolume(1);
    LoadVolume(2);

    int caller = CALLER_EVENT;
    int reti = vsaMapManagerSUT->EnableInternalAccess(0, caller);
    EXPECT_EQ(reti, 0);

    caller = CALLER_NOT_EVENT;
    reti = vsaMapManagerSUT->EnableInternalAccess(1, caller);
    EXPECT_EQ(reti, 0);
    delete mockVolumeManager;
}

} // namespace ibofos
