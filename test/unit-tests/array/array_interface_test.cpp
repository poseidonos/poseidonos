#include "src/array/array_interface.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array/rebuild/rebuild_target_mock.h"
#include "test/unit-tests/array/service/io_recover/i_recover_mock.h"
#include "test/unit-tests/array/service/io_translator/i_translator_mock.h"

namespace pos
{
TEST(ArrayInterface, AddTranslator_testIfITranslatorHasBeenAdded)
{
    // Given
    ArrayInterface aIntf;
    MockITranslator mockITrans;

    // When 1: we add a new key
    aIntf.AddTranslator(PartitionType::USER_DATA, &mockITrans);

    // Then 1: the key/value should be inserted
    map<PartitionType, ITranslator*> pTypeToITrans = aIntf.GetTranslator();
    ASSERT_EQ(1, pTypeToITrans.size());
    ASSERT_EQ(&mockITrans, pTypeToITrans[PartitionType::USER_DATA]);

    // When 2: let's try to add different translator with the same key
    MockITranslator mockITransVer2;
    aIntf.AddTranslator(PartitionType::USER_DATA, &mockITransVer2);

    // Then 2: the same key shouldn't be upserted
    ASSERT_EQ(1, pTypeToITrans.size());
    ASSERT_EQ(&mockITrans /* note that this isn't mockITransVer2*/, pTypeToITrans[PartitionType::USER_DATA]);
}

TEST(ArrayInterface, AddRecover_testIfIRecoverHasBeenAdded)
{
    // Given
    ArrayInterface aIntf;
    MockIRecover mockIRecover;

    // When 1: we add a new key
    aIntf.AddRecover(PartitionType::USER_DATA, &mockIRecover);

    // Then 1: the key/value should be inserted
    map<PartitionType, IRecover*> pTypeToIRecover = aIntf.GetRecover();
    ASSERT_EQ(1, pTypeToIRecover.size());
    ASSERT_EQ(&mockIRecover, pTypeToIRecover[PartitionType::USER_DATA]);

    // When 2: let's try to add different recover with the same key
    MockIRecover mockIRecoverVer2;
    aIntf.AddRecover(PartitionType::USER_DATA, &mockIRecoverVer2);

    // Then 2: the same key shouldn't be upserted
    ASSERT_EQ(1, pTypeToIRecover.size());
    ASSERT_EQ(&mockIRecover, pTypeToIRecover[PartitionType::USER_DATA]);
}

TEST(ArrayInterface, AddRebuildTarget_testIfRebuildTargetHasBeenAdded)
{
    // Given
    ArrayInterface aIntf;
    MockRebuildTarget mockRebuildTarget(PartitionType::USER_DATA);

    // When
    aIntf.AddRebuildTarget(&mockRebuildTarget);

    // Then
    std::list<RebuildTarget*> rebuildTargets = aIntf.GetRebuildTargets();
    ASSERT_EQ(1, rebuildTargets.size());
    ASSERT_EQ(&mockRebuildTarget, rebuildTargets.front());
}

TEST(ArrayInterface, ClearInterface_testIfAllMembersAreCleared)
{
    // Given
    ArrayInterface aIntf;
    MockITranslator mockITrans;
    MockIRecover mockIRecover;
    MockRebuildTarget mockRebuildTarget(PartitionType::USER_DATA);

    aIntf.AddTranslator(PartitionType::USER_DATA, &mockITrans);
    aIntf.AddRecover(PartitionType::USER_DATA, &mockIRecover);
    aIntf.AddRebuildTarget(&mockRebuildTarget);

    // When
    aIntf.ClearInterface();

    // Then
    ASSERT_EQ(0, aIntf.GetTranslator().size());
    ASSERT_EQ(0, aIntf.GetRecover().size());
    ASSERT_EQ(0, aIntf.GetRebuildTargets().size());
}

} // namespace pos
