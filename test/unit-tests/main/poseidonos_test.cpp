#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/main/poseidonos_mock.h"

namespace pos
{
TEST(Poseidonos, Init_)
{
}

TEST(Poseidonos, Run_)
{
}

TEST(Poseidonos, Terminate_)
{
}

TEST(Poseidonos, _InitDebugInfo_)
{
}

TEST(Poseidonos, _InitSpdk_)
{
}

TEST(Poseidonos, _InitAffinity_)
{
}

TEST(Poseidonos, _InitIOInterface_)
{
}

TEST(Poseidonos, _LoadVersion_)
{
}

TEST(Poseidonos, _SetPerfImpact_)
{
}

TEST(Poseidonos, _LoadConfiguration_)
{
}

TEST(Poseidonos, _RunCLIService_)
{
}

TEST(Poseidonos, _SetupThreadModel_)
{
}

TEST(Poseidonos, InitTraceExporter_)
{
    // Given
    MockIbofos ibofos;

    // When
    int ret = ibofos.InitTraceExporter("/home/pos/ibofos/src/../bin/poseidonos");

    // Then
    ASSERT_EQ(EID(SUCCESS), ret);
}

} // namespace pos
