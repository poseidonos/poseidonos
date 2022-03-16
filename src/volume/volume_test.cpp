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

#include "volume_test.h"

#include "src/array_mgmt/array_manager.h"
#include "src/include/pos_event_id.h"


TEST_F(VolumeTest, CreateNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string vol_name = "mytestvol";

    int res = volMgr->Create(vol_name, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(SUCCESS));
    // cleanup for next test
    volMgr->Delete(vol_name);
}

TEST_F(VolumeTest, TryToCreateDuplicatedVolumeTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string vol_name = "testvol";
    volMgr->Create(vol_name, SIZE, 0, 0);
    int res = volMgr->Create(vol_name, SIZE, 0, 0); // create again for same volume name
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_DUPLICATED));

    // cleanup for next test
    volMgr->Delete(vol_name);
}

TEST_F(VolumeTest, TryToCreateInvalidVolumeNameTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string shortvol = "1";
    int res = volMgr->Create(shortvol, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_TOO_SHORT));

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
    res = volMgr->Create(longvol, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_TOO_LONG));

    std::string blankvol = "        ";
    res = volMgr->Create(blankvol, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    std::string startWithBlank = " 44444";
    res = volMgr->Create(startWithBlank, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    std::string endWithBlank = "44444 ";
    res = volMgr->Create(endWithBlank, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    std::string specialChar = "mySpeci@lVolume";
    res = volMgr->Create(specialChar, SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));
}

TEST_F(VolumeTest, TryToCreateInvalidSizeVolume)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string vol_name = "invalidvol";

    int res = volMgr->Create(vol_name, NOT_ALIGNED_SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_SIZE_NOT_ALIGNED));

    vol_name = "oversizeVol";
    res = volMgr->Create(vol_name, BIG_SIZE, 0, 0);
    EXPECT_TRUE(res == EID(CREATE_VOL_SIZE_EXCEEDED));
}

TEST_F(VolumeTest, DeleteNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string vol_name = "testvol";
    volMgr->Create(vol_name, SIZE,  0, 0);
    int res = volMgr->Delete(vol_name);
    EXPECT_TRUE(res == EID(SUCCESS));
}

TEST_F(VolumeTest, TryToDeleteInvalidVolumeTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    // create
    std::string vol_name = "testvol";
    volMgr->Create(vol_name, SIZE, 0, 0);

    // delete wrong
    {
        std::string wrong_vol_name = "wrongvol";
        int res = volMgr->Delete(wrong_vol_name);
        EXPECT_TRUE(res == EID(VOL_NOT_FOUND));
    }

    // cleanup for next test
    volMgr->Delete(vol_name);
}

TEST_F(VolumeTest, UpdateVolumeQoSNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string vol_name = "testvol";
    volMgr->Create(vol_name, SIZE, 0, 0);
    int res = volMgr->UpdateQoS(vol_name, 100, 200, 10, 20);
    EXPECT_TRUE(res == EID(SUCCESS));

    // cleanup for next test
    volMgr->Delete(vol_name);
}

TEST_F(VolumeTest, TryToUpdateInvalidVolumeQoSTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string vol_name = "testvol";
    volMgr->Create(vol_name, SIZE, 0, 0);
    int res = volMgr->UpdateQoS("invalidvol", 100, 200, 10, 20);
    EXPECT_TRUE(res == EID(VOL_NOT_FOUND));

    // cleanup for next test
    volMgr->Delete(vol_name);
}

TEST_F(VolumeTest, RenameNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string old_vol_name = "old_testvol";
    std::string new_vol_name = "new_testvol";
    volMgr->Create(old_vol_name, SIZE, 0, 0);
    int res = volMgr->Rename(old_vol_name, new_vol_name);
    EXPECT_TRUE(res == EID(SUCCESS));
    res = volMgr->VolumeID(new_vol_name);
    EXPECT_TRUE(res == 0);

    // cleanup for next test
    volMgr->Delete(new_vol_name);
}

TEST_F(VolumeTest, TryToUpdateInvalidVolumeNameTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    std::string oldVolName = "old_testvol";
    volMgr->Create(oldVolName, SIZE, 0, 0);

    std::string invalidCharVol = "fsdla?";
    int res = volMgr->Rename(oldVolName, invalidCharVol);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    std::string blankVol = "    ";
    res = volMgr->Rename(oldVolName, blankVol);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    std::string shortVol = "a";
    res = volMgr->Rename(oldVolName, shortVol);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_TOO_SHORT));

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

    res = volMgr->Rename(oldVolName, longVol);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_TOO_LONG));

    std::string beginSpace = " avolvol";
    res = volMgr->Rename(oldVolName, beginSpace);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    std::string endSpace = "bvolvol ";
    res = volMgr->Rename(oldVolName, endSpace);
    EXPECT_TRUE(res == EID(CREATE_VOL_NAME_NOT_ALLOWED));

    volMgr->Delete(oldVolName);
}

TEST_F(VolumeTest, MountVolumeNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    // create
    std::string volName = "testvol";
    volMgr->Create(volName, SIZE, 0, 0);
    int res = volMgr->Mount(volName, NQN);
    EXPECT_TRUE(res == EID(SUCCESS));
    // cleanup for next test
    volMgr->Unmount(volName);
    volMgr->Delete(volName);
}

TEST_F(VolumeTest, TryToDeleteMountedVolumeTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    // create
    std::string volName = "testvol";
    volMgr->Create(volName, SIZE, 0, 0);
    volMgr->Mount(volName, NQN);
    int res = volMgr->Delete(volName);
    EXPECT_TRUE(res == EID(DELETE_VOL_MOUNTED_VOL_CANNOT_BE_DELETED));
}

TEST_F(VolumeTest, TryToMountInvalidVolumeTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    int res = volMgr->Mount("notexist", NQN);
    EXPECT_TRUE(res == EID(VOL_NOT_FOUND));
}

TEST_F(VolumeTest, UnmountVolumeNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    // create
    std::string volName = "testvol";
    volMgr->Create(volName, SIZE, 0, 0);
    volMgr->Mount(volName, NQN);
    int res = volMgr->Unmount(volName);
    EXPECT_TRUE(res == EID(SUCCESS));
    // cleanup for next test
    volMgr->Delete(volName);
}

TEST_F(VolumeTest, TryToUnmountNotMountedVolumeTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    // create
    std::string volName = "testvol";
    volMgr->Create(volName, SIZE, 0, 0);
    int res = volMgr->Unmount(volName);
    EXPECT_TRUE(res == EID(UNMOUNT_VOL_ALREADY_UNMOUNTED));
    // cleanup for next test
    volMgr->Delete(volName);
}

TEST_F(VolumeTest, TryToUnmountInvalidVolumeTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    int res = volMgr->Unmount("notexist");
    EXPECT_TRUE(res == EID(VOL_NOT_FOUND));
}

TEST_F(VolumeTest, VolumeCountBasicTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    volMgr->Create("testvol1", SIZE, 0, 0);
    EXPECT_EQ(1, volMgr->GetVolumeCount());
    volMgr->Create("testvol2", SIZE, 0, 0);
    EXPECT_EQ(2, volMgr->GetVolumeCount());
    volMgr->Create("testvol3", SIZE, 0, 0);
    EXPECT_EQ(3, volMgr->GetVolumeCount());
    volMgr->Delete("testvol2");
    EXPECT_EQ(2, volMgr->GetVolumeCount());
    volMgr->Create("testvol2", SIZE, 0, 0);
    EXPECT_EQ(3, volMgr->GetVolumeCount());
    volMgr->Create("testvol4", SIZE, 0, 0);
    EXPECT_EQ(4, volMgr->GetVolumeCount());
    volMgr->Delete("testvol2");
    EXPECT_EQ(3, volMgr->GetVolumeCount());
    volMgr->Delete("testvol1");
    EXPECT_EQ(2, volMgr->GetVolumeCount());
    volMgr->Delete("testvol3");
    EXPECT_EQ(1, volMgr->GetVolumeCount());
    volMgr->Delete("testvol4");
    EXPECT_EQ(0, volMgr->GetVolumeCount());
}

TEST_F(VolumeTest, VolumeIDNormalTest)
{
    pos::IVolumeManager* volMgr = pos::VolumeServiceSingleton::Instance()->GetVolumeManager(ARRAY_NAME);
    volMgr->Create("testvol_id0", SIZE, 0, 0);
    EXPECT_EQ(0, volMgr->VolumeID("testvol_id0"));

    volMgr->Create("testvol_id1", SIZE, 0, 0);
    EXPECT_EQ(1, volMgr->VolumeID("testvol_id1"));

    volMgr->Create("testvol_id2", SIZE, 0, 0);
    EXPECT_EQ(2, volMgr->VolumeID("testvol_id2"));

    volMgr->Delete("testvol_id1");

    volMgr->Create("testvol_id1_again", SIZE, 0, 0);
    EXPECT_EQ(1, volMgr->VolumeID("testvol_id1_again"));

    volMgr->Delete("testvol_id0");

    volMgr->Create("testvol_id0_again", SIZE, 0, 0);
    EXPECT_EQ(0, volMgr->VolumeID("testvol_id0_again"));
}
