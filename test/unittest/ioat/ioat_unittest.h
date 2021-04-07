#include <atomic>

#include "gtest/gtest.h"
#include "src/device/ioat_api.h"

using namespace std;

namespace ibofos
{
class IoatTest : public testing::Test
{
protected:
    IoatTest(void);
    virtual ~IoatTest(void);

    void InitializeBuffer(void);
    static void Verify(void* argument);
    static void HandleSubmitCopy(void* arg1, void* arg2);

    static const uint32_t BUFFER_SIZE = 4096;
    static const uint8_t PATTERN = 0xA;

    static void* src;
    static void* dst;
    static void* ori;
    static atomic<bool> done;
    static bool verifyResult;
};
}
