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

#include "src/metafs/mim/mio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/metafs/config/metafs_config_manager_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"
#include "test/unit-tests/metafs/mim/mpio_allocator_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
using MioStateHandler = std::function<bool(MioState)>;

class MioStateExecuteEntryTexture : public ::testing::Test
{
public:
    MioStateExecuteEntryTexture(void)
    {
    }
    ~MioStateExecuteEntryTexture(void)
    {
    }
    virtual void SetUp(void) override
    {
        entry = nullptr;
        result = false;
    }
    virtual void TearDown(void) override
    {
        if (entry)
            delete entry;
    }
    bool CallbackTest(MioState st)
    {
        if (state == st)
            result = true;

        return true;
    }

protected:
    MioState state;
    MioStateHandler handler;
    MioState expNextState;
    bool result;

    MioStateExecuteEntry* entry;
};

TEST_F(MioStateExecuteEntryTexture, Constructor)
{
    state = MioState::Complete;
    handler = nullptr;
    expNextState = MioState::Error;

    entry = new MioStateExecuteEntry(state, handler, expNextState);
}

TEST_F(MioStateExecuteEntryTexture, GetState_testIfTheStateSetByTheConstructorCanBeReturned)
{
    state = MioState::Complete;
    handler = nullptr;
    expNextState = MioState::Error;

    entry = new MioStateExecuteEntry(state, handler, expNextState);

    EXPECT_EQ(entry->GetState(), MioState::Complete);
    EXPECT_EQ(entry->GetExpNextState(), MioState::Error);
}

TEST_F(MioStateExecuteEntryTexture, DispatchHandler_testIfTheCallbackSetByTheConstructorCanBeCalled)
{
    state = MioState::Complete;
    handler = std::bind(&MioStateExecuteEntryTexture::CallbackTest, this, std::placeholders::_1);
    expNextState = MioState::Error;

    EXPECT_FALSE(result);

    entry = new MioStateExecuteEntry(state, handler, expNextState);
    entry->DispatchHandler(MioState::Complete);

    EXPECT_TRUE(result);
}

TEST(Mio, GetId_testIfTheUniqueIdIsUnique)
{
    NiceMock<MockMetaFsConfigManager> conf(nullptr);
    ON_CALL(conf, GetMpioPoolCapacity).WillByDefault(Return(1));

    MpioAllocator* mpioAllocator = new NiceMock<MockMpioAllocator>(&conf);

    Mio m1(mpioAllocator);
    Mio m2(mpioAllocator);

    EXPECT_NE(m1.GetId(), m2.GetId());

    delete mpioAllocator;
}

TEST(Mio, GetFileType_testIfTheFileTypeCanBeReturned)
{
    NiceMock<MockMetaFsConfigManager> conf(nullptr);
    ON_CALL(conf, GetMpioPoolCapacity).WillByDefault(Return(1));

    MpioAllocator* mpioAllocator = new NiceMock<MockMpioAllocator>(&conf);
    Mio mio(mpioAllocator);

    EXPECT_EQ(mio.GetFileType(), MetaFileType::General);

    MetaFileExtent extent;
    MetaFileContext ctx;
    ctx.fileType = MetaFileType::Journal;
    NiceMock<MockMetaFsIoRequest> req;
    ON_CALL(req, GetStartLpn).WillByDefault(Return(0));
    req.reqType = MetaIoRequestType::Read;
    req.fileCtx = &ctx;
    req.extents = &extent;
    req.extentsCount = 1;

    mio.Setup(&req, 0, nullptr);
    EXPECT_EQ(mio.GetFileType(), ctx.fileType);

    delete mpioAllocator;
}
} // namespace pos
