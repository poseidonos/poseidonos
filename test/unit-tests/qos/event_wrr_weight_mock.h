#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/event_wrr_weight.h"

namespace pos
{
class MockQosEventWrrWeight : public QosEventWrrWeight
{
public:
    using QosEventWrrWeight::QosEventWrrWeight;
};

} // namespace pos
