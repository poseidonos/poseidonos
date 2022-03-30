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

#include "src/metafs/mim/meta_async_cb_cxt.h"

#include <gtest/gtest.h>

#include <functional>

namespace pos
{
class MetaAsyncCbCxtTester
{
public:
    MetaAsyncCbCxtTester(void)
    : result(false)
    {
    }
    void TestCallbackFunction(void* data)
    {
        int* obj = reinterpret_cast<int*>(data);
        *obj = 10;
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

TEST(MetaAsyncCbCxt, Init_testIfMssAioDataWillBeSet)
{
    MetaAsyncCbCxtTester tester;
    MetaAsyncCbCxt cxt;
    AsyncCallback cb = std::bind(&MetaAsyncCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    int data = 0;

    cxt.Init(&data, cb);

    EXPECT_EQ(cxt.GetAsycCbCxt(), &data);
}

TEST(MetaAsyncCbCxt, InvokeCallback_checkIfTheCallbackWillBeCalled)
{
    MetaAsyncCbCxtTester tester;
    MetaAsyncCbCxt cxt;
    AsyncCallback cb = std::bind(&MetaAsyncCbCxtTester::TestCallbackFunction, &tester, std::placeholders::_1);
    int data = 0;

    cxt.Init(&data, cb);
    cxt.InvokeCallback();

    EXPECT_EQ(data, 10);
    EXPECT_TRUE(tester.GetResult());
}
} // namespace pos
