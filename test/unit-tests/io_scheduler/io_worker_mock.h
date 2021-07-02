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
    MOCK_METHOD(void, EnqueueUbio, (UbioSmartPtr), (override));
    MOCK_METHOD(uint32_t, RemoveDevice, (UblockSharedPtr), (override));
};

} // namespace pos
