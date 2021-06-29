#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_scheduler/io_worker.h"

namespace pos
{
class MockIOWorker : public IOWorker
{
public:
    using IOWorker::IOWorker;
};

} // namespace pos
