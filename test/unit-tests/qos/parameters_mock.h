#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameters.h"

namespace pos
{
class MockQosParameters : public QosParameters
{
public:
    using QosParameters::QosParameters;
};

} // namespace pos
