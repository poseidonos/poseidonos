#include "src/cpu_affinity/affinity_viewer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/cpu_affinity/cpu_set_generator.h"
#include "test/unit-tests/cpu_affinity/affinity_config_parser_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"

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

TEST(AffinityViewer, Print_numaAvailable)
{
    // Given
    NiceMock<MockAffinityConfigParser>* mockAffinityConfigParser = new NiceMock<MockAffinityConfigParser>();
    ON_CALL(*mockAffinityConfigParser, IsStringDescripted()).WillByDefault(Return(false));
    ON_CALL(*mockAffinityConfigParser, GetDescriptions()).WillByDefault(ReturnRef(TEST_CORE_DESCRIPTIONS));
    testing::Mock::AllowLeak(mockAffinityConfigParser);
    NiceMock<MockAffinityManager>* mockAffinityManager = new NiceMock<MockAffinityManager>(mockAffinityConfigParser);
    AffinityViewer affinityViewer;

    // When : call Print
    affinityViewer.Print(mockAffinityManager);

    // Then : Do nothing
}

} // namespace pos
