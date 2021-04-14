#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/accel_engine_api.h"

namespace pos
{
class MockIoatArgument : public IoatArgument
{
public:
    using IoatArgument::IoatArgument;
};

class MockAccelEngineApi : public AccelEngineApi
{
public:
    using AccelEngineApi::AccelEngineApi;
};

} // namespace pos
