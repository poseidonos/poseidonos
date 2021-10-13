#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameter_queue.h"

namespace pos
{
class MockParameterQueue : public ParameterQueue
{
public:
    using ParameterQueue::ParameterQueue;
};

} // namespace pos
