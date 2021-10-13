#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_context.h"

namespace pos
{
class MockQosContext : public QosContext
{
public:
    using QosContext::QosContext;
    MOCK_METHOD(bool, GetApplyCorrection, ());
};

} // namespace pos
