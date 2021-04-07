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

#include "reverse_map_test.h"

#include "src/allocator/stripe.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"

namespace ibofos
{
void
ReverseMapTest::SetUp(void)
{
    ReverseMapPack::SetPageSize();
    ReverseMapPack::SetNumPagesUT(UT_REVMAP_PAGES_PER_STRIPE);
    reverseMapPack = new ReverseMapPack();
    reverseMapPack->Init(UT_WBLSID);
}

void
ReverseMapTest::TearDown(void)
{
    delete reverseMapPack;
    reverseMapPack = nullptr;
}

MetaFileIntf*
ReverseMapTest::_PrepareFile(void)
{
    MetaFileIntf* revMapWholefile = new MockFileIntf("RevMapWhole.bin");
    if (revMapWholefile->DoesFileExist() == false)
    {
        uint64_t fileSize = ReverseMapPack::GetfileSizePerStripe() * MAX_UT_VSID;
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::REVMAP_FILE_SIZE,
            "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
            ReverseMapPack::GetfileSizePerStripe(), MAX_UT_VSID, fileSize);

        int ret = revMapWholefile->Create(fileSize);
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REVMAP_FILE_SIZE,
                "RevMapWhole file Create failed, ret:{}", ret);
            assert(false);
        }
    }
    revMapWholefile->Open();

    return revMapWholefile;
}

TEST_F(ReverseMapTest, GetFileSizePerStripe)
{
    uint64_t fileSizePerStripe = reverseMapPack->GetfileSizePerStripe();
    std::cout << "fileSizePerStripe:" << fileSizePerStripe << std::endl;

    EXPECT_EQ(fileSizePerStripe, DEFAULT_REVMAP_PAGE_SIZE * UT_REVMAP_PAGES_PER_STRIPE);
}

TEST_F(ReverseMapTest, LinkVSID)
{
    int ret = 0;

    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_LT(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, UnLinkVSID)
{
    int ret = 0;

    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    ret = reverseMapPack->UnLinkVsid();
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    ret = reverseMapPack->UnLinkVsid();
    EXPECT_LT(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, SetAndFlush)
{
    int ret = 0;

    // Prepare Mock file
    MetaFileIntf* revMapFile = _PrepareFile();
    reverseMapPack->SetRevMapFileUT(revMapFile);

    // Set some revesemap data
    ret = reverseMapPack->SetReverseMapEntry(UT_OFFSET, UT_RBA, UT_VOLUMEID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Link VSID
    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Flush
    Stripe stripeDummy;
    ret = reverseMapPack->Flush(&stripeDummy, nullptr);
    EXPECT_EQ(ret, RET_INT_SUCCESS);
    usleep(1000);

    ret = revMapFile->Close();
    EXPECT_EQ(ret, RET_INT_SUCCESS);
}

TEST_F(ReverseMapTest, LoadAndGet)
{
    int ret = 0;

    // Prepare Mock file
    MetaFileIntf* revMapFile = _PrepareFile();
    reverseMapPack->SetRevMapFileUT(revMapFile);

    // Link VSID
    ret = reverseMapPack->LinkVsid(UT_VSID);
    EXPECT_EQ(ret, RET_INT_SUCCESS);

    // Load
    ret = reverseMapPack->Load();
    EXPECT_EQ(ret, RET_INT_SUCCESS);
    usleep(1000);

    // Get
    BlkAddr rba;
    uint32_t volumeId;
    std::tie(rba, volumeId) = reverseMapPack->GetReverseMapEntry(UT_OFFSET);
    EXPECT_EQ(rba, UT_RBA);
    EXPECT_EQ(volumeId, UT_VOLUMEID);

    ret = revMapFile->Close();
    EXPECT_EQ(ret, RET_INT_SUCCESS);
}

} // namespace ibofos
