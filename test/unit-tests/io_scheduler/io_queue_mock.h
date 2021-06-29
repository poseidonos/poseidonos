#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/io_queue.h"

namespace pos
{
class MockUbioArray : public UbioArray
{
public:
    using UbioArray::UbioArray;
};

class MockIOQueue : public IOQueue
{
public:
    using IOQueue::IOQueue;
};

} // namespace pos
