#include "src/array/partition/partition_manager.h"

#include <gtest/gtest.h>

#include "src/array/partition/partition_services.h"
#include "src/array/device/array_device.h"
#include "src/include/array_config.h"
#include "src/include/array_device_state.h"
#include "src/helper/calc/calc.h"
#include "test/unit-tests/array/partition/partition_services_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/utils/mock_builder.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"

using ::testing::_;
using ::testing::Return;
namespace pos
{

TEST(PartitionManager, PartitionManager_testConstructor)
{
    // Given
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();

    // When
    PartitionManager pm;

    // Then
}

TEST(PartitionManager, GetSizeInfo_testIfNullIsReturnedForUninitializedPartition)
{
    // Given
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    PartitionManager pm;

    // When
    auto actual = pm.GetSizeInfo(PartitionType::META_NVM);

    // Then
    ASSERT_EQ(nullptr, actual);
}
} // namespace pos
