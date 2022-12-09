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

#include "src/metafs/include/metafs_aiocb_cxt.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFsAioCbCxt, CheckTagId)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetTagId(), 0);

    cb.SetTagId(10);

    EXPECT_EQ(cb.GetTagId(), 10);
}

TEST(MetaFsAioCbCxt, CheckError_Positive)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.CheckIOError(), true);

    MfsError err = { 0, false };
    cb.SetErrorStatus(err);

    EXPECT_EQ(cb.CheckIOError(), false);
}

TEST(MetaFsAioCbCxt, CheckError_Negative0)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.CheckIOError(), true);

    MfsError err = { 1, false };
    cb.SetErrorStatus(err);

    EXPECT_EQ(cb.CheckIOError(), true);
}

TEST(MetaFsAioCbCxt, CheckError_Negative1)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.CheckIOError(), true);

    MfsError err = { 0, true };
    cb.SetErrorStatus(err);

    EXPECT_EQ(cb.CheckIOError(), true);
}

TEST(MetaFsAioCbCxt, CheckCallback)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    cb.SetCallbackCount(10);
    cb.InvokeCallback();
}

TEST(MetaFsAioCbCxt, CheckOpcode)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetOpCode(), MetaFsIoOpcode::Read);
}

TEST(MetaFsAioCbCxt, CheckFd)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetFD(), 0);
}

TEST(MetaFsAioCbCxt, CheckOffset)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetOffset(), 0);
}

TEST(MetaFsAioCbCxt, CheckSize)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetByteSize(), 0);
}

TEST(MetaFsAioCbCxt, CheckBuffer)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_TRUE(cb.GetBuffer() == nullptr);
}
} // namespace pos
