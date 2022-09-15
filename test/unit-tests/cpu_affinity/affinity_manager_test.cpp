#include "src/cpu_affinity/affinity_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/cpu_affinity/cpu_set_generator.h"
#include "src/include/core_const.h"
#include "test/unit-tests/cpu_affinity/affinity_config_parser_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
const CoreDescriptionArray TEST_CORE_DESCRIPTIONS =
    {
        CoreDescription{CoreType::REACTOR, {1, 0}, "0"},
        CoreDescription{CoreType::UDD_IO_WORKER, {1, 0}, "1"},
        CoreDescription{CoreType::EVENT_SCHEDULER, {1, 0}, "2"},
        CoreDescription{CoreType::EVENT_WORKER, {3, 0}, "3-5"},
        CoreDescription{CoreType::GENERAL_USAGE, {1, 0}, "6"},
        CoreDescription{CoreType::QOS, {1, 0}, "7"},
        CoreDescription{CoreType::META_SCHEDULER, {1, 0}, "8"},
        CoreDescription{CoreType::META_IO, {2, 0}, "9-10"},
        CoreDescription{CoreType::AIR, {1, 0}, "11"},
};

TEST(AffinityManager, AffinityManager_Stack_UseString)
{
    // Given : AffinityConfigParser::IsStringDescripted returns true
    NiceMock<MockAffinityConfigParser>* mockAffinityConfigParser = new NiceMock<MockAffinityConfigParser>();
    ON_CALL(*mockAffinityConfigParser, IsStringDescripted()).WillByDefault(Return(false)); // true -> false. workaround to avoid invoking exit(1) call on a host not having enough cpu cores.
    ON_CALL(*mockAffinityConfigParser, GetDescriptions()).WillByDefault(ReturnRef(TEST_CORE_DESCRIPTIONS));
    testing::Mock::AllowLeak(mockAffinityConfigParser);

    // Wnen : Create affinity Manager on stack
    AffinityManager affinityManager(mockAffinityConfigParser);

    // Then : Do nothing
}

TEST(AffinityManager, AffinityManager_Heap_NotUseString)
{
    // Given : AffinityConfigParser::IsStringDescripted returns false
    NiceMock<MockAffinityConfigParser>* mockAffinityConfigParser = new NiceMock<MockAffinityConfigParser>();
    ON_CALL(*mockAffinityConfigParser, IsStringDescripted()).WillByDefault(Return(false));
    ON_CALL(*mockAffinityConfigParser, GetDescriptions()).WillByDefault(ReturnRef(TEST_CORE_DESCRIPTIONS));
    testing::Mock::AllowLeak(mockAffinityConfigParser);

    // Wnen : Create affinity Manager on heap
    AffinityManager* affinityManager = new AffinityManager(mockAffinityConfigParser);

    // Then : Release memory
    delete affinityManager;
}

TEST(AffinityManager, SetGeneralAffinitySelf_Call)
{
    // Given :
    NiceMock<MockAffinityConfigParser>* mockAffinityConfigParser = new NiceMock<MockAffinityConfigParser>();
    ON_CALL(*mockAffinityConfigParser, IsStringDescripted()).WillByDefault(Return(false)); // true -> false. workaround to avoid invoking exit(1) call on a host not having enough cpu cores.
    ON_CALL(*mockAffinityConfigParser, GetDescriptions()).WillByDefault(ReturnRef(TEST_CORE_DESCRIPTIONS));
    testing::Mock::AllowLeak(mockAffinityConfigParser);

    // Wnen : Create affinity Manager on stack
    AffinityManager affinityManager(mockAffinityConfigParser);
    affinityManager.SetGeneralAffinitySelf();

    // Then : Do nothing
}

TEST(AffinityManager, GetCpuSet_ReturnCpuSetArray)
{
    // Given : Set CpuSet
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    CPU_SET(0, &cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetCpuSet
    cpu_set_t expected = cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)];
    cpu_set_t actual = affinityManager.GetCpuSet(CoreType::EVENT_WORKER);

    // Then : Check cpu_set_t value
    ASSERT_TRUE(CPU_EQUAL(&expected, &actual));
}

TEST(AffinityManager, GetReactorCPUSetString_ReturnReactorBitString)
{
    // Given : Set CpuSet(reactor : 0x2)
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::EVENT_REACTOR)]);
    CPU_SET(0, &cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    CPU_SET(1, &cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetReactorCPUSetString
    string cpuString = affinityManager.GetReactorCPUSetString();
    uint32_t actual = std::atoi(cpuString.c_str());
    uint32_t expected = 3;

    // Then : Check cpu_set_t value
    ASSERT_EQ(expected, actual);
}

TEST(AffinityManager, GetMasterReactorCore_ReturnValidCore)
{
    // Given : Set CpuSet(reactor : 0x2)
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    CPU_SET(0, &cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    CPU_SET(1, &cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetMasterReactorCore
    uint32_t actual = affinityManager.GetMasterReactorCore();
    int expected = 0;

    // Then : Check cpu_set_t value
    ASSERT_EQ(expected, actual);
}

TEST(AffinityManager, GetMasterReactorCore_ReturnInvalidCore)
{
    // Given : Set CpuSet(reactor)
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::REACTOR)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetMasterReactorCore
    uint32_t actual = affinityManager.GetMasterReactorCore();
    int expected = INVALID_CORE;

    // Then : Check cpu_set_t value
    ASSERT_EQ(expected, actual);
}

TEST(AffinityManager, GetEventWorkerSocket_ReturnMaxEventCoreNuma)
{
    // Given : Set CpuSet(event worker : 0x2)
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    CPU_SET(0, &cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    CPU_SET(1, &cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetEventWorkerSocket
    uint32_t actual = affinityManager.GetEventWorkerSocket();
    int expected = 0;

    // Then : Check return value
    ASSERT_EQ(expected, actual);
}

TEST(AffinityManager, GetEventWorkerSocket_ReturnFail)
{
    // Given : Set CpuSet(event worker : 0x3)
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetEventWorkerSocket
    uint32_t actual = affinityManager.GetEventWorkerSocket();
    int expected = UINT32_MAX;

    // Then : Check return value
    ASSERT_EQ(expected, actual);
}

TEST(AffinityManager, GetTotalCore_ReturnTotalCore)
{
    // Given : set TOTAL_COUNT : 5
    CpuSetArray cpuArray;
    uint32_t totalCount = 5;

    // When : call GetTotalCore
    AffinityManager affinityManager(totalCount, cpuArray);
    uint32_t actual = affinityManager.GetTotalCore();

    // Then : check return total count
    ASSERT_EQ(totalCount, actual);
}

TEST(AffinityManager, GetCoreCount_ReturnCoreCount)
{
    // Given : Set CpuSet(event worker : 0x3)
    CpuSetArray cpuArray;
    CPU_ZERO(&cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    CPU_SET(0, &cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    CPU_SET(1, &cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    CPU_SET(2, &cpuArray[static_cast<uint32_t>(CoreType::EVENT_WORKER)]);
    AffinityManager affinityManager(8, cpuArray);

    // When : call GetEventWorkerSocket
    uint32_t actual = affinityManager.GetCoreCount(CoreType::EVENT_WORKER);
    int expected = 3;

    // Then : Check return value
    ASSERT_EQ(expected, actual);
}

} // namespace pos
