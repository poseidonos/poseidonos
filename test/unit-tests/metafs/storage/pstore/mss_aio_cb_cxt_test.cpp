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

#include "src/metafs/storage/pstore/mss_aio_cb_cxt.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>

#include "test/unit-tests/metafs/storage/pstore/mss_aio_data_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MssAioCbCxtTester
{
public:
    MssAioCbCxtTester(void)
    : result(false)
    {
    }
    void TestCallbackFunction(void* data)
    {
        MssAioData* obj = reinterpret_cast<MssAioData*>(data);
        obj->SetError(10);
        SetResult(true);
    }
    void SetResult(const bool flag)
    {
        result = flag;
    }
    bool GetResult(void) const
    {
        return result;
    }

private:
    bool result;
};

TEST(MssAioCbCxt, Init_testIfMssAioDataWillBeSet)
{
    MssAioCbCxtTester tester;
    MssAioCbCxt cxt;
    AsyncCallback cb = std::bind(&MssAioCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    MssAioData data;

    cxt.Init(&data, cb);

    EXPECT_EQ(cxt.GetIoContext(), &data);
}

TEST(MssAioCbCxt, callback_checkIfTheCallbackWillBeCalled)
{
    MssAioCbCxtTester tester;
    MssAioCbCxt cxt;
    AsyncCallback cb = std::bind(&MssAioCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    MssAioData data;
    data.SetError(0);

    cxt.Init(&data, cb);

    EXPECT_EQ(data.GetError(), 0);
    EXPECT_FALSE(tester.GetResult());

    cxt.InvokeCallback();

    EXPECT_EQ(data.GetError(), 10);
    EXPECT_TRUE(tester.GetResult());
}

TEST(MssAioCbCxt, SaveIOStatus_testIfTheMockWillBeCalled)
{
    MssAioCbCxtTester tester;
    MssAioCbCxt cxt;
    AsyncCallback cb = std::bind(&MssAioCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    NiceMock<MockMssAioData> data;

    EXPECT_CALL(data, SetError);

    cxt.Init(&data, cb);
    cxt.SaveIOStatus(10);
}

TEST(MssAioCbCxt, GetArrayId_testIfTheMethodWillReturnTheArrayId)
{
    MssAioCbCxtTester tester;
    MssAioCbCxt cxt;
    AsyncCallback cb = std::bind(&MssAioCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    NiceMock<MockMssAioData> data;

    EXPECT_CALL(data, GetArrayId).WillOnce(Return(10));

    cxt.Init(&data, cb);

    EXPECT_EQ(cxt.GetArrayId(), 10);
}

TEST(MssAioCbCxt, GetIoContext_testIfTheMethodWillReturnTheDataObject)
{
    MssAioCbCxtTester tester;
    MssAioCbCxt cxt;
    AsyncCallback cb = std::bind(&MssAioCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    MssAioData data;

    cxt.Init(&data, cb);

    EXPECT_EQ(cxt.GetIoContext(), &data);
}
} // namespace pos
