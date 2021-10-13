#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/io_queue.h"

namespace pos
{
template<typename T>
class MockIoQueue : public IoQueue<T>
{
public:
    using IoQueue::IoQueue;
};

} // namespace pos
