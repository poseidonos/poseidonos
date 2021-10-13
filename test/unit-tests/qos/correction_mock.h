#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/correction.h"

namespace pos
{
class MockQosCorrection : public QosCorrection
{
public:
    using QosCorrection::QosCorrection;
};

} // namespace pos
