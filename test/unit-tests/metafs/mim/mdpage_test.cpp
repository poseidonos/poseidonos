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

#include "src/metafs/mim/mdpage.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/crc.hpp>
#include <cstring>

#include "src/include/memory.h"
#include "src/metafs/config/metafs_config.h"
#include "src/metafs/mim/mdpage_control_info.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MDpageFixture : public ::testing::Test
{
public:
    MDpageFixture(void)
    : page(nullptr),
      info(nullptr),
      buf(nullptr)
    {
        buf = pos::Memory<TEST_PAGE_SIZE>::Alloc(1);

        info = new NiceMock<MockIArrayInfo>();
        ON_CALL(*info, GetName()).WillByDefault(Return(""));
        ON_CALL(*info, GetIndex()).WillByDefault(Return(0));

        page = new MDPage(buf);
        EXPECT_NE(page->GetDataBuffer(), nullptr) << "Memory allocation is failed";
    }

    virtual ~MDpageFixture()
    {
        delete info;
        delete page;

        pos::Memory<TEST_PAGE_SIZE>::Free(buf);
    }

    virtual void SetUp(void) override
    {
        page->AttachControlInfo();
        page->BuildControlInfo(LPN_NUMBER, FD, ARRAY_ID, SIGNATURE);
    }

    virtual void TearDown(void) override
    {
        page->ClearControlInfo();
    }

protected:
    static const size_t TEST_PAGE_SIZE = 4096;
    const MetaLpnType LPN_NUMBER = 10;
    const FileDescriptorType FD = 9;
    const uint64_t SIGNATURE = 123456789;
    const int ARRAY_ID = 1;

    MDPage* page;
    NiceMock<MockIArrayInfo>* info;
    void* buf;
};

TEST_F(MDpageFixture, GetMfsSignature_testIfTheSignatureMatchesExpectedValue)
{
    uint32_t expected = MDPageControlInfo::MDPAGE_CTRL_INFO_SIG;
    EXPECT_EQ(page->GetMfsSignature(), expected);
}

TEST_F(MDpageFixture, IsValidSignature_testIfThePageHasValidSignatures)
{
    EXPECT_TRUE(page->IsValidSignature(SIGNATURE));
}

TEST_F(MDpageFixture, GetDataBuffer_testIfTheBufferAddressMatchesExpectedValue)
{
    EXPECT_EQ(page->GetDataBuffer(), (uint8_t*)buf);
}

TEST_F(MDpageFixture, GetDefaultDataChunkSize_testIfTheChunkSizeMatchesExpectedValue)
{
    size_t expected = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    EXPECT_EQ(page->GetDefaultDataChunkSize(), expected);
}

TEST_F(MDpageFixture, GetCrcCoveredSize_testIfTheCrcCoveredSizeMatchesExpectedValue)
{
    size_t expected = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES - sizeof(uint32_t);
    EXPECT_EQ(page->GetCrcCoveredSize(), expected);
}

TEST_F(MDpageFixture, GenerateCrcFromDataBuffer_testIfGeneratedCrcValueMatchesWhatExpected)
{
    // fill
    std::memset(buf, 0xA55AA55A, page->GetCrcCoveredSize());

    // gen crc
    boost::crc_32_type result;
    result.reset();
    result.process_bytes(buf, page->GetCrcCoveredSize());

    // check
    EXPECT_EQ(page->GenerateCrcFromDataBuffer(), result.checksum());

    // fill to make invalid
    std::memset(buf, 0x0, page->GetDefaultDataChunkSize());

    // check
    EXPECT_NE(page->GenerateCrcFromDataBuffer(), result.checksum());
}

TEST_F(MDpageFixture, CheckDataIntegrity_testIfDataIntegrityIsGood)
{
    // fill
    std::memset(buf, 0xA55AA55A, page->GetDefaultDataChunkSize());

    // update
    page->BuildControlInfo(LPN_NUMBER, FD, ARRAY_ID, SIGNATURE);

    // check
    EXPECT_EQ(0, page->CheckDataIntegrity(LPN_NUMBER, FD));
}

TEST_F(MDpageFixture, CheckDataIntegrity_testIfLpnNumberIsNotMatched)
{
    // fill
    std::memset(buf, 0xA55AA55A, page->GetDefaultDataChunkSize());

    // update
    page->BuildControlInfo(LPN_NUMBER, FD, ARRAY_ID, SIGNATURE);

    // check
    EXPECT_EQ(EID(MFS_INVALID_META_LPN), page->CheckDataIntegrity(LPN_NUMBER + 1, FD));
}

TEST_F(MDpageFixture, CheckDataIntegrity_testIfFileDescriptorIsNotMatched)
{
    // fill
    std::memset(buf, 0xA55AA55A, page->GetDefaultDataChunkSize());

    // update
    page->BuildControlInfo(LPN_NUMBER, FD, ARRAY_ID, SIGNATURE);

    // check
    EXPECT_EQ(EID(MFS_INVALID_FILE_DESCRIPTOR), page->CheckDataIntegrity(LPN_NUMBER, FD + 1));
}

TEST_F(MDpageFixture, CheckDataIntegrity_testIfCrcIsNotMatched)
{
    // fill
    std::memset(buf, 0xA55AA55A, page->GetDefaultDataChunkSize());

    // update
    page->BuildControlInfo(LPN_NUMBER, FD, ARRAY_ID, SIGNATURE);

    // fill to make invalid
    std::memset(buf, 0x0, page->GetDefaultDataChunkSize());

    // check
    EXPECT_EQ(EID(MFS_INVALID_CRC), page->CheckDataIntegrity(LPN_NUMBER, FD));
}

TEST_F(MDpageFixture, CheckDataIntegrity_testIfEvenIfCrcIsNotMatched)
{
    // fill
    std::memset(buf, 0xA55AA55A, page->GetDefaultDataChunkSize());

    // update
    page->BuildControlInfo(LPN_NUMBER, FD, ARRAY_ID, SIGNATURE);

    // check
    EXPECT_EQ(EID(SUCCESS), page->CheckDataIntegrity(LPN_NUMBER, FD));

    // fill to make invalid
    std::memset(buf, 0x0, page->GetDefaultDataChunkSize());

    // check
    EXPECT_EQ(EID(SUCCESS), page->CheckDataIntegrity(LPN_NUMBER, FD, true));
}
} // namespace pos
