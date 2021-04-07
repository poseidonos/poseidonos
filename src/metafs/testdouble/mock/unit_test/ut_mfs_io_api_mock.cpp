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

#include <gtest/gtest.h>

#include "mfs.h"

// https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md
// https://github.com/google/googletest/blob/master/googlemock/docs/cheat_sheet.md

using testing::InitGoogleMock;

using ::testing::Return;

TEST(MockMFSIOAPITest, Read)
{
    MetaFsMgrClass metaFsMgr;

    void* buf = nullptr;
    MetaFsReturnCode<IBOF_EVENT_ID> rc;
    rc.sc = IBOF_EVENT_ID::ParamErr;
    EXPECT_CALL(metaFsMgr.io, Read(0, buf))
        .WillOnce(Return(rc));

    rc = metaFsMgr.io.Read(0, buf);
    EXPECT_EQ(rc.sc, IBOF_EVENT_ID::ParamErr);

    rc.sc = IBOF_EVENT_ID::FileNotOpened;
    EXPECT_CALL(metaFsMgr.io, Read(0, buf))
        .WillOnce(Return(rc));

    rc = metaFsMgr.io.Read(0, buf);
    EXPECT_EQ(rc.sc, IBOF_EVENT_ID::FileNotOpened);
}

int
main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
