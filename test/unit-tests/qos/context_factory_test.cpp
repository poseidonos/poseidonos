#include "src/qos/context_factory.h"

#include <gtest/gtest.h>

#include "src/qos/qos_context.h"

namespace pos
{
TEST(ContextFactory, CreateQosContext_Test)
{
    QosContext* qosContext = ContextFactory::CreateQosContext();
    ASSERT_NE(qosContext, NULL);
}

} // namespace pos
