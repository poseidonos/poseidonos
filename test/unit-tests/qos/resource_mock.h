#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/resource.h"

namespace pos
{
class MockQosResource : public QosResource
{
public:
    using QosResource::QosResource;
};

} // namespace pos
