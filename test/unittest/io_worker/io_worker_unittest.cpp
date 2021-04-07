// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "io_worker_unittest.h"
#include "src/device/mock_device.h"
namespace {

// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>


}  // namespace

// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?
namespace ibofos
{
IOWorkerTest::IOWorkerTest(void)
: ioWorker(nullptr)
{
}

IOWorkerTest::~IOWorkerTest(void)
{
    if (ioWorker)
    {
        delete ioWorker;
        ioWorker = nullptr;
    }
}
TEST_F(IOWorkerTest, DeviceManagement)
{
    cpu_set_t cpuSet;
    CPU_SET(1, &cpuSet);
    ioWorker = new IOWorker(cpuSet, false);

    std::vector<UBlockDevice*> deviceList;
    for (int i = 0 ; i < TEST_DEVICE_COUNT ; i++)
    {
        MockDevice* blockDevice = new MockDevice ("testDevice", TEST_DEVICE_SIZE);
        deviceList.push_back(blockDevice);
    }
    int deviceCount = ioWorker->AddDevices(&deviceList);
    int expectedCount = TEST_DEVICE_COUNT;
    EXPECT_EQ(deviceCount, expectedCount);

    bool added = true;
    for (auto iter = deviceList.begin() ; iter != deviceList.end() ; ++iter)
    {
        added = ioWorker->HasDevice(*iter);
        if (added == false)
        {
            break;
        }
    }
    EXPECT_EQ(added, true);

    int processedCount = 0;
    for (auto & iter : deviceList)
    {
        int remainDeviceCount = ioWorker->RemoveDevice(iter);
        processedCount++;
        int totalCount = TEST_DEVICE_COUNT;
        EXPECT_EQ(remainDeviceCount + processedCount, totalCount);
        bool existance = ioWorker->HasDevice(iter);
        EXPECT_EQ(existance, false);
    }
}

TEST_F(IOWorkerTest, DoIO)
{
    cpu_set_t cpuSet;
    CPU_SET(1, &cpuSet);
    ioWorker = new IOWorker(cpuSet, false);

    std::vector<UBlockDevice*> deviceList;
    for (int i = 0 ; i < TEST_DEVICE_COUNT ; i++)
    {
        MockDevice* blockDevice = new MockDevice ("testDevice", TEST_DEVICE_SIZE);
        deviceList.push_back(blockDevice);
    }
    ioWorker->AddDevices(&deviceList);

    for (auto iter = deviceList.begin() ; iter != deviceList.end() ; ++iter)
    {
        Ubio writeUbio(nullptr, DATA_SIZE);
        writeUbio.dev = *iter;
        writeUbio.dir = UbioDir::Write;
        writeUbio.syncDone = false;
        memset(writeUbio.GetBuffer(0, 0), 'a', DATA_SIZE * Ubio::BYTES_PER_UNIT);
        ioWorker->SyncIO(&writeUbio);

        Ubio readUbio(nullptr, DATA_SIZE);
        readUbio.dev = *iter;
        readUbio.dir = UbioDir::Read;
        readUbio.syncDone = false;
        memset(readUbio.GetBuffer(0, 0), 'b', DATA_SIZE * Ubio::BYTES_PER_UNIT);
        ioWorker->SyncIO(&readUbio);
        bool correct = (0 == memcmp(writeUbio.GetBuffer(0, 0), readUbio.GetBuffer(0, 0), DATA_SIZE * Ubio::BYTES_PER_UNIT));
        EXPECT_EQ(correct, true);
    }
}
}
GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
