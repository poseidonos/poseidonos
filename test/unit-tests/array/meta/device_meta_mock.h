#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/meta/device_meta.h"

namespace pos
{
class MockDeviceMeta : public DeviceMeta
{
public:
    using DeviceMeta::DeviceMeta;
};

} // namespace pos
