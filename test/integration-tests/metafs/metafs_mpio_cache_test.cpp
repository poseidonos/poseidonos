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

#include <string>

#include "test/integration-tests/metafs/lib/test_metafs.h"

using namespace std;

namespace pos
{
class MetaFsMpioCacheTest : public TestMetaFs
{
public:
    MetaFsMpioCacheTest(void)
    : TestMetaFs()
    {
    }
    virtual ~MetaFsMpioCacheTest(void)
    {
    }

protected:
};

TEST_F(MetaFsMpioCacheTest, VerifyData_testIfTheDataWillBeCorrect_SyncIo)
{
    const size_t GRANULARITY_BYTE_SIZE = 52;
    const size_t COUNT = 1000;

    SetGranularity(GRANULARITY_BYTE_SIZE);

    // write
    EXPECT_TRUE(WritePatternSync(0, COUNT));

    // read and verify
    EXPECT_TRUE(VerifyPattern(0, COUNT));
}

TEST_F(MetaFsMpioCacheTest, VerifyData_testIfTheDataWillBeCorrect_AsyncIo)
{
    const size_t GRANULARITY_BYTE_SIZE = 52;
    const size_t LPN_COUNT = 1000;
    // 2s
    const size_t WAIT_TIMEOUT = 2000;

    SetGranularity(GRANULARITY_BYTE_SIZE);

    CreateBuffers(LPN_COUNT);

    // write
    EXPECT_TRUE(WritePattern(0, LPN_COUNT));

    // wait
    EXPECT_TRUE(WaitForDone(WAIT_TIMEOUT));

    // read and verify
    EXPECT_TRUE(VerifyPattern(0, LPN_COUNT));

    DeleteBuffers();
}

TEST_F(MetaFsMpioCacheTest, VerifyDataInMultiArray_testIfTheDataWillBeCorrect_AsyncIo)
{
    const size_t GRANULARITY_BYTE_SIZE = 52;
    const size_t LPN_COUNT = 1000;
    // 2s
    const size_t WAIT_TIMEOUT = 2000;

    SetGranularity(GRANULARITY_BYTE_SIZE);

    CreateBuffers(LPN_COUNT);

    // write
    for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
    {
        EXPECT_TRUE(WritePattern(arrayId, LPN_COUNT));
    }

    EXPECT_TRUE(WaitForDone(WAIT_TIMEOUT));

    // read and verify
    for (int arrayId = 0; arrayId < MetaFsTestFixture::ARRAY_COUNT; ++arrayId)
    {
        EXPECT_TRUE(VerifyPattern(arrayId, LPN_COUNT));
    }

    DeleteBuffers();
}
} // namespace pos
