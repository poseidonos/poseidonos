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

#include "mfs_core_mgr_test.h"

// try to mount before filecreation
TEST_F(MetaFsCoreMgrTest, MountBeforeFSCreation)
{
    MetaFsControlReqMsg reqMsg;
    reqMsg.reqType = MetaFsControlReqType::Mount; // open -> active state transition

    IBOF_EVENT_ID sc;
    // metaFsCoreMgrClass::handleMountReq()
    // Open State : MetaFsStateProcedure::_ProcessSystemState_Open()
    sc = mfsCoreMgr.ProcessNewReq(reqMsg);

    EXPECT_EQ(sc, IBOF_EVENT_ID::MFS_FILE_NOT_FOUND);
}

TEST_F(MetaFsCoreMgrTest, HandleFSCreateReq)
{
    MetaFsControlReqMsg reqMsg;
    reqMsg.reqType = MetaFsControlReqType::FileSysCreate;

    IBOF_EVENT_ID sc;
    sc = mfsCoreMgr.ProcessNewReq(reqMsg);

    EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
}

TEST_F(MetaFsCoreMgrTest, _HandleMountReq)
{
    MetaFsControlReqMsg reqMsg;
    reqMsg.reqType = MetaFsControlReqType::Mount; // open -> active sate transition

    IBOF_EVENT_ID sc;
    // metafscoremgrclass::handlemountreq()
    // open state : metafsstateprocedure::processsystemstate_open()
    // metastorae for each media-> loadmbri@metaregionclass-> mvm open-> mim bringup
    sc = mfsCoreMgr.ProcessNewReq(reqMsg);

    EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
}

TEST_F(MetaFsCoreMgrTest, _HandleUnmountReq)
{
    MetaFsControlReqMsg reqMsg;
    reqMsg.reqType = MetaFsControlReqType::Unmount; // quiesec -> shutdown state

    IBOF_EVENT_ID sc;
    // MetaFsStateProcedure::_ProcessSystemState_Shutdown()
    // MBR save-> mvm close-> metastorage close
    sc = mfsCoreMgr.ProcessNewReq(reqMsg);

    EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
}

// Repetedly test for 'mount -> unmount'
TEST_F(MetaFsCoreMgrTest, HandleMount_UnMount)
{
    int loop = 10;

    MetaFsControlReqMsg reqMsg;

    for (int cnt = 0; cnt < loop; cnt++)
    {
        reqMsg.reqType = MetaFsControlReqType::Mount; // open -> active sate transition

        IBOF_EVENT_ID sc;
        sc = mfsCoreMgr.ProcessNewReq(reqMsg);

        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);

        reqMsg.reqType = MetaFsControlReqType::Unmount; // quiesec -> shutdown state
        sc = mfsCoreMgr.ProcessNewReq(reqMsg);

        EXPECT_EQ(sc, IBOF_EVENT_ID::SUCCESS);
    }
}
