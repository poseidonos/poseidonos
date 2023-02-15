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
    vector<AteData*> emptyCandidates;

    // When
    int actual = PbrSelector::Select(emptyCandidates);

    // Then
    ASSERT_EQ(EID(PBR_VOTING_NO_CANDIDATE), actual);
}

TEST(PbrSelector, Select_testIfNoneOfCandidatesElected)
{
    // Given
    vector<AteData*> candidates;
    AteData* ateData = new AteData();
    ateData->lastUpdatedDateTime = 10000; // not interesting
    ateData->arrayUuid = "uuid"; // not interesting
    candidates.push_back(ateData);

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
    vector<AteData*> candidates;
    AteData* ateData1 = new AteData();
    ateData1->lastUpdatedDateTime = 50000;
    ateData1->arrayUuid = "uuid";
    ateData1->arrayName = "loser";
    AteData* ateData2 = new AteData();
    ateData2->lastUpdatedDateTime = 30000;
    ateData2->arrayUuid = "uuid";
    ateData2->arrayName = "winner";
    AteData* ateData3 = new AteData();
    ateData3->lastUpdatedDateTime = 30000;
    ateData3->arrayUuid = "uuid";
    ateData3->arrayName = "winner";
    candidates.push_back(ateData1);
    candidates.push_back(ateData2);
    candidates.push_back(ateData3);

    // When
    int actual = PbrSelector::Select(candidates);

    // Then
    ASSERT_EQ(1, candidates.size()); // candidates who failed to win should be removed
    ASSERT_EQ("winner", candidates.front()->arrayName);
    ASSERT_EQ(30000, candidates.front()->lastUpdatedDateTime);

    // Clean up
    delete candidates.front();
}

}  // namespace pbr
