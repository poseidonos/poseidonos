#include "gtest/gtest.h"
#include "src/scheduler/io_worker.h"
namespace ibofos
{
class IOWorkerTest : public testing::Test
{
protected:
    static const int TEST_DEVICE_SIZE = 100 * 1024 * 1024;
    static const int TEST_DEVICE_COUNT = 10;
    static const int DATA_SIZE = 16;
    IOWorkerTest(void);
    virtual ~IOWorkerTest(void);
    IOWorker* ioWorker;
};
}
