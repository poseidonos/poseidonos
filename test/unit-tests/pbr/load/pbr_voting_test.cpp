#include <gtest/gtest.h>
#include "src/pbr/load/pbr_voting.h"
 
namespace pbr
{
TEST(PbrVoting, Poll_testIfSingleCandidateInSingleArray)
{
    // Given
    PbrVoting voting;
    string uuid = "array-uuid-01";
    AteData* data1 = new AteData();
    data1->lastUpdatedDateTime = 50000;
    data1->arrayUuid = uuid;
    data1->arrayName = "data1";
    
    // When
    voting.Vote(data1);

    // Then
    auto winners = voting.Poll();
    ASSERT_EQ(1, winners.size());
    ASSERT_EQ(uuid, (*winners.begin()).first);
    ASSERT_EQ(data1, (*winners.begin()).second);

    // Clean up
    delete data1;
}

TEST(PbrVoting, Poll_testIfSingleCandidatePerArray)
{
    // Given
    PbrVoting voting;
    string uuid1 = "array-uuid-01";
    string uuid2 = "array-uuid-02";
    AteData* data1 = new AteData();
    data1->lastUpdatedDateTime = 50000;
    data1->arrayUuid = uuid1;
    data1->arrayName = "data1";
    AteData* data2 = new AteData();
    data2->lastUpdatedDateTime = 70000;
    data2->arrayUuid = uuid2;
    data2->arrayName = "data2";

    // When
    voting.Vote(data1);
    voting.Vote(data2);

    // Then
    auto winners = voting.Poll();
    ASSERT_EQ(2, winners.size());
    ASSERT_EQ(data1, winners.find(uuid1)->second);
    ASSERT_EQ(data2, winners.find(uuid2)->second);

    // Clean up
    delete data2;
    delete data1;
}

TEST(PbrVoting, Poll_testIfTwoCandidatesInSingleArrayAndNumOfVotesIsDifferent)
{
    // Given
    PbrVoting voting;
    string uuid = "array-uuid-01";
    AteData* data1 = new AteData();
    data1->lastUpdatedDateTime = 70000;
    data1->arrayUuid = uuid;
    data1->arrayName = "data1";
    AteData* data2 = new AteData();
    data2->lastUpdatedDateTime = 50000;
    data2->arrayUuid = uuid;
    data2->arrayName = "data2";
    
    // When
    // data2 gets two votes
    voting.Vote(data2);
    voting.Vote(data2); 

    // data1 gets one vote
    voting.Vote(data1);

    // Then
    // data2, which has obtained a lot of votes, becomes winner
    auto winners = voting.Poll();
    ASSERT_EQ(1, winners.size());
    ASSERT_EQ(uuid, (*winners.begin()).first);
    ASSERT_EQ(data2, winners.find(uuid)->second);

    // Clean up
    delete data2;
    delete data1;
}

TEST(PbrVoting, Poll_testIfTwoCandidatesInSingleArrayAndNumOfVotesIsSame)
{
    // Given
    PbrVoting voting;
    string uuid = "array-uuid-01";
    AteData* data1 = new AteData();
    data1->lastUpdatedDateTime = 50000;
    data1->arrayUuid = uuid;
    data1->arrayName = "data1";
    AteData* data2 = new AteData();
    data2->lastUpdatedDateTime = 70000;
    data2->arrayUuid = uuid;
    data2->arrayName = "data2";

    // When
    // both data1 and data2 gets two votes
    voting.Vote(data1);
    voting.Vote(data1); 
    voting.Vote(data2);
    voting.Vote(data2);

    // Then
    // data2 with the latest date becomes winner
    auto winners = voting.Poll();
    ASSERT_EQ(1, winners.size());
    ASSERT_EQ(uuid, (*winners.begin()).first);
    ASSERT_EQ(data2, winners.find(uuid)->second);

    // Clean up
    delete data2;
    delete data1;
}

TEST(PbrVoting, Poll_testIfTwoCandidatesPerArrays)
{
    // Given
    PbrVoting voting;
    string uuid1 = "array-uuid-01";
    string uuid2 = "array-uuid-02";

    // candidates for uuid1
    AteData* data1 = new AteData();
    data1->lastUpdatedDateTime = 50000;
    data1->arrayUuid = uuid1;
    data1->arrayName = "data1";
    AteData* data2 = new AteData();
    data2->lastUpdatedDateTime = 70000;
    data2->arrayUuid = uuid1;
    data2->arrayName = "data2";

    // candidates for uuid2
    AteData* data3 = new AteData();
    data3->lastUpdatedDateTime = 110000;
    data3->arrayUuid = uuid2;
    data3->arrayName = "data3";
    AteData* data4 = new AteData();
    data4->lastUpdatedDateTime = 90000;
    data4->arrayUuid = uuid2;
    data4->arrayName = "data4";

    // When
    // In uuid1, two candidates are tied
    voting.Vote(data1);
    voting.Vote(data1); 
    voting.Vote(data2);
    voting.Vote(data2);

    // In uuid2, two candidates have different votes
    voting.Vote(data3);
    voting.Vote(data4);
    voting.Vote(data4);

    // Then
    auto winners = voting.Poll();
    // one winner per array (total two arrays)
    ASSERT_EQ(2, winners.size());

    // In uuid1, data2 with the latest date becomes winner
    ASSERT_EQ(data2, winners.find(uuid1)->second);
    // In uuid2, data4, which has obtained a lot of votes, becomes winner
    ASSERT_EQ(data4, winners.find(uuid2)->second);

    // Clean up
    delete data4;
    delete data3;
    delete data2;
    delete data1;
}

}  // namespace pbr
