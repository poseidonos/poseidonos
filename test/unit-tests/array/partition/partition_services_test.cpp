#include "src/array/partition/partition_services.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array/rebuild/rebuild_target_mock.h"
#include "test/unit-tests/array/service/io_recover/i_recover_mock.h"
#include "test/unit-tests/array/service/io_translator/i_translator_mock.h"

namespace pos
{
TEST(PartitionServices, AddTranslator_testIfITranslatorHasBeenAdded)
{
    // Given
    PartitionServices svc;
    MockITranslator mockITrans;

    // When 1: we add a new key
    svc.AddTranslator(PartitionType::USER_DATA, &mockITrans);

    // Then 1: the key/value should be inserted
    map<PartitionType, ITranslator*> pTypeToITrans = svc.GetTranslator();
    ASSERT_EQ(1, pTypeToITrans.size());
    ASSERT_EQ(&mockITrans, pTypeToITrans[PartitionType::USER_DATA]);

    // When 2: let's try to add different translator with the same key
    MockITranslator mockITransVer2;
    svc.AddTranslator(PartitionType::USER_DATA, &mockITransVer2);

    // Then 2: the same key shouldn't be upserted
    ASSERT_EQ(1, pTypeToITrans.size());
    ASSERT_EQ(&mockITrans /* note that this isn't mockITransVer2*/, pTypeToITrans[PartitionType::USER_DATA]);
}

TEST(PartitionServices, AddRecover_testIfIRecoverHasBeenAdded)
{
    // Given
    PartitionServices svc;
    MockIRecover mockIRecover;

    // When 1: we add a new key
    svc.AddRecover(PartitionType::USER_DATA, &mockIRecover);

    // Then 1: the key/value should be inserted
    map<PartitionType, IRecover*> pTypeToIRecover = svc.GetRecover();
    ASSERT_EQ(1, pTypeToIRecover.size());
    ASSERT_EQ(&mockIRecover, pTypeToIRecover[PartitionType::USER_DATA]);

    // When 2: let's try to add different recover with the same key
    MockIRecover mockIRecoverVer2;
    svc.AddRecover(PartitionType::USER_DATA, &mockIRecoverVer2);

    // Then 2: the same key shouldn't be upserted
    ASSERT_EQ(1, pTypeToIRecover.size());
    ASSERT_EQ(&mockIRecover, pTypeToIRecover[PartitionType::USER_DATA]);
}

TEST(PartitionServices, AddRebuildTarget_testIfRebuildTargetHasBeenAdded)
{
    // Given
    PartitionServices svc;
    MockRebuildTarget mockRebuildTarget(true);

    // When
    svc.AddRebuildTarget(&mockRebuildTarget);

    // Then
    std::list<RebuildTarget*> rebuildTargets = svc.GetRebuildTargets();
    ASSERT_EQ(1, rebuildTargets.size());
    ASSERT_EQ(&mockRebuildTarget, rebuildTargets.front());
}

TEST(PartitionServices, ClearInterface_testIfAllMembersAreCleared)
{
    // Given
    PartitionServices svc;
    MockITranslator mockITrans;
    MockIRecover mockIRecover;
    MockRebuildTarget mockRebuildTarget(PartitionType::USER_DATA);

    svc.AddTranslator(PartitionType::USER_DATA, &mockITrans);
    svc.AddRecover(PartitionType::USER_DATA, &mockIRecover);
    svc.AddRebuildTarget(&mockRebuildTarget);

    // When
    svc.Clear();

    // Then
    ASSERT_EQ(0, svc.GetTranslator().size());
    ASSERT_EQ(0, svc.GetRecover().size());
    ASSERT_EQ(0, svc.GetRebuildTargets().size());
}

} // namespace pos
