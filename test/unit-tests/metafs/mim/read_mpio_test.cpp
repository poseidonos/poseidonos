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

#include "src/metafs/mim/read_mpio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::NiceMock;

namespace pos
{
class ReadMpioTester : public ReadMpio
{
public:
    explicit ReadMpioTester(void* mdPageBuf)
    : ReadMpio(mdPageBuf, false)
    {
        mss = new NiceMock<MockMetaStorageSubsystem>(0);
    }
    ~ReadMpioTester(void)
    {
        delete mss;
    }
    bool HandleError(MpAioState expNextState)
    {
        return ReadMpio::_HandleError(expNextState);
    }
    void Setup(MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO)
    {
        EXPECT_CALL(*mss, IsAIOSupport);
        ReadMpio::Setup(mpioIoInfo, partialIO, forceSyncIO, mss);
    }

private:
    NiceMock<MockMetaStorageSubsystem>* mss;
};

TEST(ReadMpioTester, ReadMpio_testConstructor)
{
    MpioIoInfo ioInfo;
    char* buf = (char*)malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    memset(buf, 0, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    ReadMpioTester* mpio = new ReadMpioTester(buf);
    mpio->Setup(ioInfo, true, false);

    EXPECT_EQ(mpio->GetCurrState(), MpAioState::Init);

    delete mpio;

    free(buf);
}

TEST(ReadMpioTester, _HandleError_testIfTheMethodChangesNextStateWhenThereIsAnError)
{
    MpioIoInfo ioInfo;
    char* buf = (char*)malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    memset(buf, 0, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    ReadMpioTester* mpio = new ReadMpioTester(buf);
    mpio->Setup(ioInfo, true, false);

    bool result = mpio->HandleError(MpAioState::Complete);

    EXPECT_EQ(result, true);
    EXPECT_EQ(mpio->GetNextState(), MpAioState::Complete);

    delete mpio;

    free(buf);
}
} // namespace pos
