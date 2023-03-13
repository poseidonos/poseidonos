#include <gtest/gtest.h>
#include "src/pbr/load/pbr_selector.h"
#include "src/include/pos_event_id.h"
#include "pbr_voting_mock.h"

using ::testing::_;
using ::testing::Return;

namespace pbr
{
TEST(PbrSelector, Select_testIfNoCandidatesReturnsError)
{
    // Given
    vector<unique_ptr<AteData>> emptyCandidates;

    // When
    int actual = PbrSelector::Select(emptyCandidates);

    // Then
    ASSERT_EQ(EID(PBR_VOTING_NO_CANDIDATE), actual);
}

TEST(PbrSelector, Select_testIfNoneOfCandidatesElected)
{
    // Given
    vector<unique_ptr<AteData>> candidates;
    unique_ptr<AteData> ateData = make_unique<AteData>();
    ateData->lastUpdatedDateTime = 10000; // not interesting
    ateData->arrayUuid = "uuid"; // not interesting
    candidates.push_back(move(ateData));

    unique_ptr<MockPbrVoting> mockPbrVoting = make_unique<MockPbrVoting>();
    map<string, AteData*> emptyWinner;
    EXPECT_CALL(*mockPbrVoting, Vote).Times(candidates.size());
    EXPECT_CALL(*mockPbrVoting, Poll).WillOnce(Return(emptyWinner));

    // When
    int actual = PbrSelector::Select(candidates, move(mockPbrVoting));

    // Then
    ASSERT_EQ(EID(PBR_VOTING_NO_WINNER), actual);
    ASSERT_EQ(0, candidates.size()); // candidates who failed to win should be removed
}

TEST(PbrSelector, Select_testIfOnlyWinnersRemainAmongCandidates)
{
    // Given
    // the candidate whose arrayName is "winner" wins with a majority of the votes.
    vector<unique_ptr<AteData>> candidates;
    unique_ptr<AteData> ateData1 = make_unique<AteData>();
    ateData1->lastUpdatedDateTime = 50000;
    ateData1->arrayUuid = "uuid";
    ateData1->arrayName = "loser";
    unique_ptr<AteData> ateData2 = make_unique<AteData>();
    ateData2->lastUpdatedDateTime = 30000;
    ateData2->arrayUuid = "uuid";
    ateData2->arrayName = "winner";
    unique_ptr<AteData> ateData3 = make_unique<AteData>();
    ateData3->lastUpdatedDateTime = 30000;
    ateData3->arrayUuid = "uuid";
    ateData3->arrayName = "winner";
    candidates.push_back(move(ateData1));
    candidates.push_back(move(ateData2));
    candidates.push_back(move(ateData3));

    // When
    int actual = PbrSelector::Select(candidates);

    // Then
    ASSERT_EQ(1, candidates.size()); // candidates who failed to win should be removed
    ASSERT_EQ("winner", candidates.front()->arrayName);
    ASSERT_EQ(30000, candidates.front()->lastUpdatedDateTime);
}

}  // namespace pbr
