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

#include "volume_test.h"

#include "src/include/ibof_event_id.h"
#include "src/volume/volume_manager.h"

TEST_F(VolumeTest, CreateNormalTest)
{
    std::string vol_name = "mytestvol";
    int res = ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);
    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(vol_name, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToCreateDuplicatedVolumeTest)
{
    std::string vol_name = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0); // create again for same volume name
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_DUPLICATED);

    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(vol_name, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToCreateInvalidVolumeNameTest)
{
    std::string shortvol = "1";
    int res = ibofos::VolumeManagerSingleton::Instance()->Create(shortvol, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_TOO_SHORT);

    std::string longvol = "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111";
    res = ibofos::VolumeManagerSingleton::Instance()->Create(longvol, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_TOO_LONG);

    std::string blankvol = "        ";
    res = ibofos::VolumeManagerSingleton::Instance()->Create(blankvol, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    std::string startWithBlank = " 44444";
    res = ibofos::VolumeManagerSingleton::Instance()->Create(startWithBlank, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    std::string endWithBlank = "44444 ";
    res = ibofos::VolumeManagerSingleton::Instance()->Create(endWithBlank, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    std::string specialChar = "mySpeci@lVolume";
    res = ibofos::VolumeManagerSingleton::Instance()->Create(specialChar, SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);
}

TEST_F(VolumeTest, TryToCreateInvalidSizeVolume)
{
    std::string vol_name = "invalidvol";

    int res = ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, NOT_ALIGNED_SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SIZE_NOT_ALIGNED);

    vol_name = "oversizeVol";
    res = ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, BIG_SIZE, ARRAY_NAME, 0, 0);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_SIZE_EXCEEDED);
}

TEST_F(VolumeTest, DeleteNormalTest)
{
    std::string vol_name = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->Delete(vol_name, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);
}

TEST_F(VolumeTest, TryToDeleteInvalidVolumeTest)
{
    // create
    std::string vol_name = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0);

    // delete wrong
    {
        std::string wrong_vol_name = "wrongvol";
        int res = ibofos::VolumeManagerSingleton::Instance()->Delete(wrong_vol_name, ARRAY_NAME);
        EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_NOT_EXIST);
    }

    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(vol_name, ARRAY_NAME);
}

TEST_F(VolumeTest, UpdateVolumeQoSNormalTest)
{
    std::string vol_name = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->UpdateQoS(vol_name, ARRAY_NAME, 100, 200);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);

    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(vol_name, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToUpdateInvalidVolumeQoSTest)
{
    std::string vol_name = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(vol_name, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->UpdateQoS("invalidvol", ARRAY_NAME, 100, 200);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_NOT_EXIST);

    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(vol_name, ARRAY_NAME);
}

TEST_F(VolumeTest, RenameNormalTest)
{
    std::string old_vol_name = "old_testvol";
    std::string new_vol_name = "new_testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(old_vol_name, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->Rename(old_vol_name, new_vol_name, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);
    res = ibofos::VolumeManagerSingleton::Instance()->VolumeID(new_vol_name);
    EXPECT_TRUE(res == 0);

    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(new_vol_name, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToUpdateInvalidVolumeNameTest)
{
    std::string oldVolName = "old_testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(oldVolName, SIZE, ARRAY_NAME, 0, 0);

    std::string invalidCharVol = "fsdla?";
    int res = ibofos::VolumeManagerSingleton::Instance()->Rename(oldVolName, invalidCharVol, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    std::string blankVol = "    ";
    res = ibofos::VolumeManagerSingleton::Instance()->Rename(oldVolName, blankVol, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    std::string shortVol = "a";
    res = ibofos::VolumeManagerSingleton::Instance()->Rename(oldVolName, shortVol, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_TOO_SHORT);

    std::string longVol = "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111"
                          "1111111111111111";

    res = ibofos::VolumeManagerSingleton::Instance()->Rename(oldVolName, longVol, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_TOO_LONG);

    std::string beginSpace = " avolvol";
    res = ibofos::VolumeManagerSingleton::Instance()->Rename(oldVolName, beginSpace, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    std::string endSpace = "bvolvol ";
    res = ibofos::VolumeManagerSingleton::Instance()->Rename(oldVolName, endSpace, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::NAME_NOT_ALLOWED);

    ibofos::VolumeManagerSingleton::Instance()->Delete(oldVolName, ARRAY_NAME);
}

TEST_F(VolumeTest, ResizeNormalTest)
{
    std::string volName = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(volName, SIZE, ARRAY_NAME, 0, 0);

    int res = ibofos::VolumeManagerSingleton::Instance()->Resize(volName, ARRAY_NAME, 2 * SIZE);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);

    res = ibofos::VolumeManagerSingleton::Instance()->Resize(volName, ARRAY_NAME, 3 * SIZE);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);
    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(volName, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToUpdateInvalidVolumeSizeTest)
{
    std::string volName = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(volName, SIZE, ARRAY_NAME, 0, 0);

    int res = ibofos::VolumeManagerSingleton::Instance()->Resize(volName, ARRAY_NAME, BIG_SIZE);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_SIZE_EXCEEDED);

    res = ibofos::VolumeManagerSingleton::Instance()->Resize(volName, ARRAY_NAME, NOT_ALIGNED_SIZE);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SIZE_NOT_ALIGNED);

    uint64_t smallSize = SIZE / 4; // UsedSize is size/2
    res = ibofos::VolumeManagerSingleton::Instance()->Resize(volName, ARRAY_NAME, smallSize);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SIZE_TOO_SMALL);
    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(volName, ARRAY_NAME);
}

TEST_F(VolumeTest, MountVolumeNormalTest)
{
    // create
    std::string volName = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(volName, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->Mount(volName, ARRAY_NAME, NQN);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);
    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Unmount(volName, ARRAY_NAME);
    ibofos::VolumeManagerSingleton::Instance()->Delete(volName, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToDeleteMountedVolumeTest)
{
    // create
    std::string volName = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(volName, SIZE, ARRAY_NAME, 0, 0);
    ibofos::VolumeManagerSingleton::Instance()->Mount(volName, ARRAY_NAME, NQN);
    int res = ibofos::VolumeManagerSingleton::Instance()->Delete(volName, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::DEL_MOUNTED_VOL);
}

TEST_F(VolumeTest, TryToMountInvalidVolumeTest)
{
    int res = ibofos::VolumeManagerSingleton::Instance()->Mount("notexist", ARRAY_NAME, NQN);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_NOT_EXIST);
}

TEST_F(VolumeTest, UnmountVolumeNormalTest)
{
    // create
    std::string volName = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(volName, SIZE, ARRAY_NAME, 0, 0);
    ibofos::VolumeManagerSingleton::Instance()->Mount(volName, ARRAY_NAME, NQN);
    int res = ibofos::VolumeManagerSingleton::Instance()->Unmount(volName, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::SUCCESS);
    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(volName, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToUnmountNotMountedVolumeTest)
{
    // create
    std::string volName = "testvol";
    ibofos::VolumeManagerSingleton::Instance()->Create(volName, SIZE, ARRAY_NAME, 0, 0);
    int res = ibofos::VolumeManagerSingleton::Instance()->Unmount(volName, ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_ALD_UNMOUNTED);
    // cleanup for next test
    ibofos::VolumeManagerSingleton::Instance()->Delete(volName, ARRAY_NAME);
}

TEST_F(VolumeTest, TryToUnmountInvalidVolumeTest)
{
    int res = ibofos::VolumeManagerSingleton::Instance()->Unmount("notexist", ARRAY_NAME);
    EXPECT_TRUE(res == (int)IBOF_EVENT_ID::VOL_NOT_EXIST);
}

TEST_F(VolumeTest, VolumeCountBasicTest)
{
    ibofos::VolumeManagerSingleton::Instance()->Create("testvol1", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(1, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Create("testvol2", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(2, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Create("testvol3", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(3, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol2", ARRAY_NAME);
    EXPECT_EQ(2, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Create("testvol2", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(3, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Create("testvol4", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(4, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol2", ARRAY_NAME);
    EXPECT_EQ(3, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol1", ARRAY_NAME);
    EXPECT_EQ(2, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol3", ARRAY_NAME);
    EXPECT_EQ(1, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol4", ARRAY_NAME);
    EXPECT_EQ(0, ibofos::VolumeManagerSingleton::Instance()->GetVolumeCount());
}

TEST_F(VolumeTest, VolumeIDNormalTest)
{
    ibofos::VolumeManagerSingleton::Instance()->Create("testvol_id0", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(0, ibofos::VolumeManagerSingleton::Instance()->VolumeID("testvol_id0"));

    ibofos::VolumeManagerSingleton::Instance()->Create("testvol_id1", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(1, ibofos::VolumeManagerSingleton::Instance()->VolumeID("testvol_id1"));

    ibofos::VolumeManagerSingleton::Instance()->Create("testvol_id2", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(2, ibofos::VolumeManagerSingleton::Instance()->VolumeID("testvol_id2"));

    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol_id1", ARRAY_NAME);

    ibofos::VolumeManagerSingleton::Instance()->Create("testvol_id1_again", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(1, ibofos::VolumeManagerSingleton::Instance()->VolumeID("testvol_id1_again"));

    ibofos::VolumeManagerSingleton::Instance()->Delete("testvol_id0", ARRAY_NAME);

    ibofos::VolumeManagerSingleton::Instance()->Create("testvol_id0_again", SIZE, ARRAY_NAME, 0, 0);
    EXPECT_EQ(0, ibofos::VolumeManagerSingleton::Instance()->VolumeID("testvol_id0_again"));
}
