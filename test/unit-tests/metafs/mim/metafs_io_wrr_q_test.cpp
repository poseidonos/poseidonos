/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/mim/metafs_io_wrr_q.h"

#include <gtest/gtest.h>

#include <vector>

#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;

namespace pos
{
class MetaFsIoWrrQFixture : public ::testing::Test
{
public:
    MetaFsIoWrrQFixture(void) = default;
    virtual ~MetaFsIoWrrQFixture(void) = default;

    virtual void SetUp(void)
    {
    }

    virtual void TearDown(void)
    {
    }

    void CheckResult(const std::vector<MetaFileType> expectedSequence)
    {
        // pop items
        for (int i = 0; i < (int)MetaFileType::MAX; i++)
        {
            auto item = multiQ.Dequeue();
            ASSERT_NE(item, nullptr);
            EXPECT_EQ(expectedSequence[i], item->GetFileType())
                << "i: " << i
                << ", item->GetFileType(): " << (int)item->GetFileType()
                << ", expectedSequence[i]: " << (int)expectedSequence[i];
            delete item;
        }
    }

    void Fill(void)
    {
        // push items
        for (int i = 0; i < (int)MetaFileType::MAX; i++)
        {
            for (int count = 0; count < TEST_ITEM_COUNT_EACH_TYPE; count++)
            {
                NiceMock<MockMetaFsIoRequest>* request = new NiceMock<MockMetaFsIoRequest>();
                ON_CALL(*request, GetFileType).WillByDefault(Return((MetaFileType)i));
                multiQ.Enqueue(request, request->GetFileType());
            }
        }
    }

    void FillExceptJournal(void)
    {
        // push items
        for (int i = 0; i < (int)MetaFileType::MAX; i++)
        {
            if (i == (int)MetaFileType::Journal)
                continue;

            for (int count = 0; count < TEST_ITEM_COUNT_EACH_TYPE; count++)
            {
                NiceMock<MockMetaFsIoRequest>* request = new NiceMock<MockMetaFsIoRequest>();
                ON_CALL(*request, GetFileType).WillByDefault(Return((MetaFileType)i));
                multiQ.Enqueue(request, request->GetFileType());
            }
        }
    }

    void Empty(void)
    {
        // delete unused items
        while (1)
        {
            MetaFsIoRequest* item = multiQ.Dequeue();
            if (!item)
                break;
            delete item;
        }
    }

protected:
    static const int TEST_ITEM_COUNT_EACH_TYPE = 10;
    std::vector<MetaFileType> expectedSequence;
    MetaFsIoWrrQ<MetaFsIoRequest*, MetaFileType> multiQ;
};

TEST_F(MetaFsIoWrrQFixture, WeightVerification_testIfDequeueSequenceMatchesWithExpectedSequence_FromSpecialPurposeMap)
{
    std::vector<MetaFileType> expectedSequence{
        MetaFileType::SpecialPurposeMap, MetaFileType::Journal, MetaFileType::Map, MetaFileType::General,
        MetaFileType::SpecialPurposeMap, MetaFileType::Journal, MetaFileType::Map, MetaFileType::General};

    Fill();

    // check sequence
    CheckResult(expectedSequence);

    Empty();
}

TEST_F(MetaFsIoWrrQFixture, WeightVerification_testIfDequeueSequenceMatchesWithExpectedSequence_FromJournal)
{
    std::vector<MetaFileType> expectedSequence{
        MetaFileType::Journal, MetaFileType::Map, MetaFileType::General, MetaFileType::SpecialPurposeMap,
        MetaFileType::Journal, MetaFileType::Map, MetaFileType::General, MetaFileType::SpecialPurposeMap};

    // precondition
    multiQ.SetStartIndex((int)MetaFileType::Journal);

    Fill();

    // check sequence
    CheckResult(expectedSequence);

    Empty();
}

TEST_F(MetaFsIoWrrQFixture, WeightVerification_testIfDequeueSequenceMatchesWithExpectedSequence_FromMap)
{
    std::vector<MetaFileType> expectedSequence{
        MetaFileType::Map, MetaFileType::Map, MetaFileType::Map,
        MetaFileType::General, MetaFileType::General, MetaFileType::General, MetaFileType::General,
        MetaFileType::SpecialPurposeMap,
        MetaFileType::Journal, MetaFileType::Journal};

    // precondition
    multiQ.SetWeight({1, 2, 3, 4});
    multiQ.SetStartIndex((int)MetaFileType::Map);

    Fill();

    // check sequence
    CheckResult(expectedSequence);

    Empty();
}

TEST_F(MetaFsIoWrrQFixture, WeightVerification_testIfDequeueSequenceMatchesWithExpectedSequence_FromGeneral)
{
    std::vector<MetaFileType> expectedSequence{
        MetaFileType::General, MetaFileType::General, MetaFileType::General, MetaFileType::General,
        MetaFileType::SpecialPurposeMap, MetaFileType::SpecialPurposeMap, MetaFileType::SpecialPurposeMap, MetaFileType::SpecialPurposeMap,
        MetaFileType::Journal, MetaFileType::Journal, MetaFileType::Journal,
        MetaFileType::Map, MetaFileType::Map};

    // precondition
    multiQ.SetWeight({4, 3, 2, 4});
    multiQ.SetStartIndex((int)MetaFileType::General);

    Fill();

    // check sequence
    CheckResult(expectedSequence);

    Empty();
}

TEST_F(MetaFsIoWrrQFixture, WeightVerification_testIfDequeueSequenceMatchesWithExpectedSequence_JournalNotExisted)
{
    std::vector<MetaFileType> expectedSequence{
        MetaFileType::General, MetaFileType::General, MetaFileType::General, MetaFileType::General,
        MetaFileType::SpecialPurposeMap, MetaFileType::SpecialPurposeMap,
        MetaFileType::Map};

    // precondition
    multiQ.SetWeight({2, 3, 1, 4});
    multiQ.SetStartIndex((int)MetaFileType::General);

    FillExceptJournal();

    // check sequence
    CheckResult(expectedSequence);

    Empty();
}
} // namespace pos
