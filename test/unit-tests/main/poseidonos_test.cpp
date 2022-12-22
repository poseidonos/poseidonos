#include <gtest/gtest.h>

#include "src/main/poseidonos.h"
#include "src/trace/otlp_factory.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/master_context/version_provider_mock.h"
#include "test/unit-tests/trace/trace_exporter_mock.h"


using ::testing::_;
using ::testing::Return;
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

TEST(Poseidonos, _InitsingletonInfo_)
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

ACTION_P(SetArg2ToBoolAndReturnSuccess, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return EID(SUCCESS);
}

ACTION_P(SetArg2ToStringAndReturnSuccess, stringValue)
{
    *static_cast<std::string*>(arg2) = stringValue;
    return EID(SUCCESS);
}

TEST(Poseidonos, _InitTraceExporter_)
{
    // Given
    Poseidonos *pos = new Poseidonos;
    MockConfigManager *cm = new MockConfigManager;
    MockVersionProvider *vp = new MockVersionProvider;
    MockTraceExporter *te = new MockTraceExporter(new OtlpFactory());

    ON_CALL(*cm, GetValue("trace", "enable", _, _)).WillByDefault(SetArg2ToBoolAndReturnSuccess(true));
    ON_CALL(*cm, GetValue("trace", "collector_endpoint", _, _)).WillByDefault(SetArg2ToStringAndReturnSuccess("localhost:1234"));
    EXPECT_CALL(*cm, GetValue).Times(2);
    EXPECT_CALL(*vp, GetVersion).WillOnce(Return("TestVersion"));
    EXPECT_CALL(*te, Init);
    EXPECT_CALL(*te, IsEnabled).WillOnce(Return(true));

    // When
    int ret = pos->_InitTraceExporter((char*)"/home/pos/ibofos/src/../bin/poseidonos", cm, vp, te);

    // Then
    ASSERT_EQ(EID(SUCCESS), ret);

    delete pos;
    delete cm;
    delete vp;
    delete te;
}

TEST(Poseidonos, _InitTraceExporterWithConfigError1_)
{
    // Given
    Poseidonos *pos = new Poseidonos;
    MockConfigManager *cm = new MockConfigManager;
    MockVersionProvider *vp = new MockVersionProvider;
    MockTraceExporter *te = new MockTraceExporter(new OtlpFactory());

    EXPECT_CALL(*cm, GetValue).WillOnce(Return(EID(CONFIG_REQUEST_KEY_ERROR)));

    // When
    int ret = pos->_InitTraceExporter((char*)"/home/pos/ibofos/src/../bin/poseidonos", cm, vp, te);

    // Then
    ASSERT_EQ(EID(TRACE_CONFIG_ERROR), ret);

    delete pos;
    delete cm;
    delete vp;
    delete te;
}

TEST(Poseidonos, _InitTraceExporterWithConfigError2_)
{
    // Given
    Poseidonos *pos = new Poseidonos;
    MockConfigManager *cm = new MockConfigManager;
    MockVersionProvider *vp = new MockVersionProvider;
    MockTraceExporter *te = new MockTraceExporter(new OtlpFactory());

    ON_CALL(*cm, GetValue("trace", "enable", _, _)).WillByDefault(SetArg2ToBoolAndReturnSuccess(true));
    ON_CALL(*cm, GetValue("trace", "collector_endpoint", _, _)).WillByDefault(Return(EID(TRACE_CONFIG_ERROR)));
    EXPECT_CALL(*cm, GetValue).Times(2);

    // When
    int ret = pos->_InitTraceExporter((char*)"/home/pos/ibofos/src/../bin/poseidonos", cm, vp, te);

    // Then
    ASSERT_EQ(EID(TRACE_CONFIG_ERROR), ret);

    delete pos;
    delete cm;
    delete vp;
    delete te;
}

TEST(Poseidonos, _InitTraceExporterWithNotEnabled_)
{
    // Given
    Poseidonos *pos = new Poseidonos;
    MockConfigManager *cm = new MockConfigManager;
    MockVersionProvider *vp = new MockVersionProvider;
    MockTraceExporter *te = new MockTraceExporter(new OtlpFactory());

    ON_CALL(*cm, GetValue("trace", "enable", _, _)).WillByDefault(SetArg2ToBoolAndReturnSuccess(false));
    EXPECT_CALL(*cm, GetValue).Times(1);

    // When
    int ret = pos->_InitTraceExporter((char*)"/home/pos/ibofos/src/../bin/poseidonos", cm, vp, te);

    // Then
    ASSERT_EQ(EID(TRACE_NOT_ENABLED), ret);

    delete pos;
    delete cm;
    delete vp;
    delete te;
}

} // namespace pos
