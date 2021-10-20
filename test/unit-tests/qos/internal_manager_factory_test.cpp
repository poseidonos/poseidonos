#include "src/qos/internal_manager_factory.h"
#include "src/qos/qos_context.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::NiceMock;

namespace pos
{
TEST(InternalManagerFactory, CreateInternalManager_Test)
{
    QosInternalManagerType internalManagerType;
    QosContext qosCtx;
    NiceMock<MockQosManager> mockQosManager;
    internalManagerType = QosInternalManager_Unknown;
    QosInternalManager* internalMgr = InternalManagerFactory::CreateInternalManager(internalManagerType, &qosCtx, &mockQosManager);
    ASSERT_EQ(internalMgr, nullptr);
}

} // namespace pos
