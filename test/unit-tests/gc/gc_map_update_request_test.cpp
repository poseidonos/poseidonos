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

#include "src/gc/gc_map_update_request.h"

#include <gtest/gtest.h>
#include <test/unit-tests/allocator/stripe/stripe_mock.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/gc_map_update_completion_mock.h>
#include <test/unit-tests/mapper/i_stripemap_mock.h>
#include <test/unit-tests/mapper/i_vsamap_mock.h>
#include <test/unit-tests/meta_service/i_meta_updater_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
namespace pos
{
class GcMapUpdateRequestTestFixture : public ::testing::Test
{
public:
    GcMapUpdateRequestTestFixture(void)
    : gcMapUpdateRequest(nullptr),
      array(nullptr)
    {
    }

    virtual ~GcMapUpdateRequestTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        testVolumeId = 1;
        arrayName = "POSArray";

        array = new NiceMock<MockIArrayInfo>;
        ON_CALL(*array, GetName).WillByDefault(Return(arrayName));
        ON_CALL(*array, GetSizeInfo(_)).WillByDefault(Return(&partitionLogicalSize));

        stripe = new NiceMock<MockStripe>();
        stripeSmartPtr = StripeSmartPtr(stripe);

        stripeMap = new NiceMock<MockIStripeMap>;
        vsaMap = new NiceMock<MockIVSAMap>;

        metaUpdater = new NiceMock<MockIMetaUpdater>;

        completionCallback = std::make_shared<MockGcMapUpdateCompletion>(stripeSmartPtr, arrayName, stripeMap, nullptr, nullptr, array, nullptr, nullptr);
    }

    virtual void
    TearDown(void)
    {
        delete gcMapUpdateRequest;
        delete array;
        delete vsaMap;
        delete stripeMap;
        delete metaUpdater;
        completionCallback = nullptr;
        stripeSmartPtr = nullptr;
    }

protected:
    GcMapUpdateRequest* gcMapUpdateRequest;

    uint32_t testVolumeId;
    std::string arrayName;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockStripe>* stripe;
    StripeSmartPtr stripeSmartPtr;

    CallbackSmartPtr completionCallback;

    NiceMock<MockIStripeMap>* stripeMap;
    NiceMock<MockIVSAMap>* vsaMap;
    NiceMock<MockIMetaUpdater>* metaUpdater;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 4,
    .blksPerStripe = 16,
    .chunksPerStripe = 4,
    .stripesPerSegment = 32,
    .totalStripes = 3200,
    .totalSegments = 100,
    };
};

TEST_F(GcMapUpdateRequestTestFixture, Execute_testIfExecutedSuccessfully)
{
    gcMapUpdateRequest = new GcMapUpdateRequest(stripeSmartPtr, completionCallback, vsaMap, array, metaUpdater);

    uint32_t stripeId = 100;
    EXPECT_CALL(*stripe, GetVsid).WillOnce(Return(stripeId));
    // given copied blk reversemap, vsa map and victim vsa map
    for (uint32_t index = 0; index < partitionLogicalSize.blksPerStripe; index++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {index, testVolumeId};
        EXPECT_CALL(*stripe, GetReverseMapEntry(index)).WillRepeatedly(Return(revMapEntry));
        VirtualBlkAddr vsa = {.stripeId = stripeId, .offset = index};
        EXPECT_CALL(*vsaMap, GetVSAInternal(testVolumeId, index, _)).WillOnce(Return(vsa));
        EXPECT_CALL(*stripe, GetVictimVsa(index)).WillOnce(Return(vsa));
        MpageList dirty;
        dirty.insert(index);
    }

    // when journal is enabled
    EXPECT_CALL(*stripe, GetWbLsid).WillOnce(Return(stripeId));
    EXPECT_CALL(*stripe, GetUserLsid).WillOnce(Return(stripeId));
    EXPECT_CALL(*metaUpdater, UpdateGcMap).Times(1);

    // then call add Gc stripe flushed log using journal write
    EXPECT_TRUE(gcMapUpdateRequest->Execute() == true);
}

} // namespace pos
