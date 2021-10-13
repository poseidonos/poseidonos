#include "src/qos/internal_manager_factory.h"
#include "src/qos/qos_context.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(InternalManagerFactory, CreateInternalManager_Test)
{
    QosInternalManagerType internalManagerType;
    QosContext qosCtx;
    internalManagerType = QosInternalManager_Unknown;
    QosInternalManager* internalMgr = InternalManagerFactory::CreateInternalManager(internalManagerType, &qosCtx);
    ASSERT_EQ(internalMgr, nullptr);
}

} // namespace pos
