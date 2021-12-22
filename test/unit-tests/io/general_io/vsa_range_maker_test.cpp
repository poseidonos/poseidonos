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

#include "src/io/general_io/vsa_range_maker.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/mapper/include/mapper_const.h"
#include "src/mapper_service/mapper_service.h"
#include "src/volume/volume_base.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class VsaRangeMakerTestFixture : public ::testing::Test
{
public:
    VsaRangeMakerTestFixture(void)
    {
    }

    virtual ~VsaRangeMakerTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        mockIVSAMap = new NiceMock<MockIVSAMap>;
        ON_CALL(*mockIVSAMap, GetVSAs(_, _, _, _)).WillByDefault([](int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray) {
            if (startRba <= 1)
            {
                vsaArray[0] = {.stripeId = 0, .offset = 0};
                vsaArray[1] = {.stripeId = 0, .offset = 1};
            }
            else if (startRba == 2)
            {
                vsaArray[0] = {.stripeId = 0, .offset = 0};
                vsaArray[1] = {.stripeId = 1, .offset = 0};
            }
            else
            {
                vsaArray[0] = UNMAP_VSA;
            }

            if (volumeId >= MAX_VOLUME_COUNT)
            {
                return -1;
            }
            return 0;
        });

        ON_CALL(*mockIVSAMap, GetVSAInternal(_, _, _)).WillByDefault([this](int volumeId, BlkAddr startRba, int& caller) {
            if (this->call_count++ == 0)
            {
                caller = NEED_RETRY;
            }
            else
            {
                caller = OK_READY;
            }
            VirtualBlkAddr tmpVsa{0, 0};
            return tmpVsa;
        });
        MapperServiceSingleton::Instance()->RegisterMapper(arrayName, arrayId, mockIVSAMap, nullptr, nullptr, nullptr, nullptr);
    }

    virtual void
    TearDown(void)
    {
        delete mockIVSAMap;
        MapperServiceSingleton::Instance()->UnregisterMapper(arrayName);
    }

protected:
    int call_count{0};
    std::string arrayName = "POSArray1";
    const uint32_t arrayId{0};
    const uint32_t VOLUME_ID{8};
    NiceMock<MockIVSAMap>* mockIVSAMap;
};

TEST_F(VsaRangeMakerTestFixture, VsaRangeMaker_Constructor_FiveArguments)
{
    //When: Create VsaRangeMaker with mapped RBA (heap)
    VsaRangeMaker* vrm = new VsaRangeMaker(VOLUME_ID, 0, 2, arrayId);
    delete vrm;
    //do nothing

    //When: Create VsaRangeMaker with mapped RBA(stack)
    VsaRangeMaker vrm2(VOLUME_ID, 0, 1, arrayId);
    //do nothing

    //When: Create VsaRangeMaker with unmapped RBA(stack)
    VsaRangeMaker vrm3(VOLUME_ID, 3, 1, arrayId);
    //do nothing

    //When: Create VsaRangeMaker with invalid Volume ID(stack)
    //Then: An exeption is thrown
    EXPECT_THROW(VsaRangeMaker vrm4(MAX_VOLUME_COUNT, 2, 2, arrayId), int);
}

TEST_F(VsaRangeMakerTestFixture, VsaRangeMaker_GetCount)
{
    //When: Create VsaRangeMaker with single RBA
    VsaRangeMaker vrm(VOLUME_ID, 0, 1, arrayId);
    //Then: VSA range count should be one
    EXPECT_EQ(vrm.GetCount(), 1);

    //When: Create VsaRangeMaker with multiple RBAs mapped to contiguous VSAs
    VsaRangeMaker vrm2(VOLUME_ID, 1, 2, arrayId);
    //Then: VSA range count should be one
    EXPECT_EQ(vrm2.GetCount(), 1);

    //When: Create VsaRangeMaker with multiple RBAs mapped to non-contiguous VSAs
    VsaRangeMaker vrm3(VOLUME_ID, 2, 2, arrayId);
    //Then: VSA range count should be two
    EXPECT_EQ(vrm3.GetCount(), 2);
}

TEST_F(VsaRangeMakerTestFixture, VsaRangeMaker_GetVsaRange)
{
    //When: Create VsaRangeMaker with single RBA
    VsaRangeMaker vrm(VOLUME_ID, 0, 1, arrayId);
    //Then: Single VSARange is returned
    VirtualBlks actual = vrm.GetVsaRange(0);
    VirtualBlks exp{{.stripeId = 0, .offset = 0}, .numBlks = 1};
    EXPECT_EQ(actual, exp);

    //When: Create VsaRangeMaker with multiple RBAs mapped to contiguous VSAs
    VsaRangeMaker vrm2(VOLUME_ID, 1, 2, arrayId);
    //Then: Single VSARange is returned
    actual = vrm2.GetVsaRange(0);
    exp = {{.stripeId = 0, .offset = 0}, .numBlks = 2};
    EXPECT_EQ(actual, exp);

    //When: Create VsaRangeMaker with multiple RBAs mapped to contiguous VSAs
    VsaRangeMaker vrm3(VOLUME_ID, 2, 2, arrayId);
    //Then: Two discrete VSARanges are returned
    actual = vrm3.GetVsaRange(0);
    exp = {{.stripeId = 0, .offset = 0}, .numBlks = 1};
    EXPECT_EQ(actual, exp);

    actual = vrm3.GetVsaRange(1);
    exp = {{.stripeId = 1, .offset = 0}, .numBlks = 1};
    EXPECT_EQ(actual, exp);
}

} // namespace pos
