#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/event_worker.h"

namespace pos
{
class MockEventWorker : public EventWorker
{
public:
    using EventWorker::EventWorker;
};

} // namespace pos
