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

#include "src/meta_file_intf/async_context.h"

#include <gtest/gtest.h>

#include <boost/format.hpp>

namespace pos
{
int TestHandler(void* data)
{
    std::cout << "TestHandler" << std::endl;
    return 1;
}

void TestCallback(AsyncMetaFileIoCtx* ctx)
{
    std::cout << "TestCallback" << std::endl;
}

TEST(AsyncMetaFileIoCtx, HandleIoComplete_NoDeathEvenIfCallbackIsNullptr)
{
    AsyncMetaFileIoCtx ctx;
    ctx.ioDoneCheckCallback = TestHandler;
    ctx.callback = nullptr;

    EXPECT_NO_FATAL_FAILURE(ctx.HandleIoComplete(nullptr));
    EXPECT_EQ(ctx.error, 1);
}

TEST(AsyncMetaFileIoCtx, HandleIoComplete_NoDeathEvenIfDonecallbackIsNullptr)
{
    AsyncMetaFileIoCtx ctx;
    ctx.ioDoneCheckCallback = nullptr;
    ctx.callback = TestCallback;

    EXPECT_NO_FATAL_FAILURE(ctx.HandleIoComplete(nullptr));
}

TEST(AsyncMetaFileIoCtx, HandleIoComplete_NoDeathEvenIfCallbacksAreNullptr)
{
    AsyncMetaFileIoCtx ctx;
    ctx.ioDoneCheckCallback = nullptr;
    ctx.callback = nullptr;

    EXPECT_NO_FATAL_FAILURE(ctx.HandleIoComplete(nullptr));
}

TEST(AsyncMetaFileIoCtx, GetError_testIfTheMethodCanReturnError)
{
    const int error = 20;
    AsyncMetaFileIoCtx ctx;
    ctx.error = error;

    EXPECT_EQ(ctx.GetError(), error);
}

TEST(AsyncMetaFileIoCtx, GetLength_testIfTheMethodCanReturnTheLength)
{
    const uint64_t length = 20;
    AsyncMetaFileIoCtx ctx;
    ctx.length = length;

    EXPECT_EQ(ctx.GetLength(), length);
}

void TestFunction(pos::AsyncMetaFileIoCtx* ctx)
{
}

TEST(AsyncMetaFileIoCtx, ToString_testIfTheMethodReturnsInternalFieldAsStringCorrectly)
{
    const MetaFsIoOpcode expectedOpcode = MetaFsIoOpcode::Read;
    const int expectedFd = 1;
    const uint64_t expectedFileOffset = 11;
    const uint64_t expectedLength = 10;
    char expectedBuffer = 'A';
    const MetaIoCbPtr expectedMetaIoCb = TestFunction;
    const int expectedError = 3;
    const MetaFileIoCbPtr expectedFileIoCb = nullptr;
    const uint32_t expectedVsid = 2;

    AsyncMetaFileIoCtx ctx;
    ctx.opcode = MetaFsIoOpcode::Read;
    ctx.fd = expectedFd;
    ctx.fileOffset = expectedFileOffset;
    ctx.length = expectedLength;
    ctx.buffer = &expectedBuffer;
    ctx.callback = expectedMetaIoCb;
    ctx.error = expectedError;
    ctx.ioDoneCheckCallback = expectedFileIoCb;
    ctx.vsid = expectedVsid;

    // opcode:1, fd:1, fileOffset:11, length:10, buffer:0x7ffe9b99fe8f, callback:not nullptr, error:3, ioDoneCheckCallback:nullptr, vsid:2
    std::string expectedStr;
    expectedStr = expectedStr.append("opcode:").append(std::to_string((int)expectedOpcode)).append(", ");
    expectedStr = expectedStr.append("fd:").append(std::to_string(expectedFd)).append(", ");
    expectedStr = expectedStr.append("fileOffset:").append(std::to_string((int)expectedFileOffset)).append(", ");
    expectedStr = expectedStr.append("length:").append(std::to_string((int)expectedLength)).append(", ");
    expectedStr = expectedStr.append("buffer:").append(((&expectedBuffer == nullptr) ? "nullptr" : "0x" + (boost::format("%x") % ((uint64_t)(uint64_t*)&expectedBuffer)).str())).append(", ");
    expectedStr = expectedStr.append("callback:").append((expectedMetaIoCb == nullptr) ? "nullptr" : "not nullptr").append(", ");
    expectedStr = expectedStr.append("error:").append(std::to_string((int)expectedError)).append(", ");
    expectedStr = expectedStr.append("ioDoneCheckCallback:").append((expectedFileIoCb == nullptr) ? "nullptr" : "not nullptr").append(", ");
    expectedStr = expectedStr.append("vsid:").append(std::to_string((int)expectedVsid));

    EXPECT_FALSE(ctx.ToString().compare(expectedStr));
}
} // namespace pos
