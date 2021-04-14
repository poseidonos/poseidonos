#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/io_worker_device_operation_queue.h"

namespace pos
{
class MockIoWorkerDeviceOperationQueue : public IoWorkerDeviceOperationQueue
{
public:
    using IoWorkerDeviceOperationQueue::IoWorkerDeviceOperationQueue;
};

} // namespace pos
