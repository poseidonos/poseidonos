#pragma once

#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

namespace pos
{
using StripeList = std::vector<StripeTestFixture>;
using Volumes = std::unordered_set<int>;

class RocksDBJournalVolumeIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBJournalVolumeIntegrationTest(void);
    virtual ~RocksDBJournalVolumeIntegrationTest(void) = default;

    StripeList WriteStripes(int numVolumes);
    void DeleteVolumes(Volumes& volumesToDelete);
    void CheckVolumeDeleteLogsWritten(Volumes& volumesToDelete);
    void ExpectReplayStripes(StripeList& writtenStripes,
        int numVolumes, Volumes& volumesToDelete);
    void ExpectReplayTail(int numVolumesWritten);

protected:
    virtual void SetUp(void) override;
    virtual void TearDown(void) override;
    JournalConfigurationBuilder builder;

};
} // namespace pos
