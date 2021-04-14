#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/io_worker_device_operation.h"

namespace pos
{
class MockIoWorkerDeviceOperation : public IoWorkerDeviceOperation
{
public:
    using IoWorkerDeviceOperation::IoWorkerDeviceOperation;
};

} // namespace pos
