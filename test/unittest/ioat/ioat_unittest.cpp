#include "ioat_unittest.h"

#include "spdk/env.h"
#include "spdk/event.h"
#include "src/io/affinity_manager.h"
#include "src/device/spdk/spdk.hpp"

using namespace ibofos;
namespace ibofos
{

void* IoatTest::src;
void* IoatTest::dst;
void* IoatTest::ori;
atomic<bool> IoatTest::done;
bool IoatTest::verifyResult;

IoatTest::IoatTest(void)
{
    src = spdk_dma_malloc(BUFFER_SIZE, BUFFER_SIZE, nullptr); 
    ori = spdk_dma_malloc(BUFFER_SIZE, BUFFER_SIZE, nullptr); 
    dst = spdk_dma_malloc(BUFFER_SIZE, BUFFER_SIZE, nullptr); 
}

IoatTest::~IoatTest(void)
{
    spdk_dma_free(src);
    spdk_dma_free(ori);
    spdk_dma_free(dst);
}

TEST_F(IoatTest,MemcpyNormalThread)
{
    InitializeBuffer();
    IoatApi::SubmitCopy(dst, src, BUFFER_SIZE, Verify, nullptr);
    while (done == false);
    EXPECT_EQ(verifyResult, true);

}

TEST_F(IoatTest,MemcpyIoReactorThread)
{
    InitializeBuffer();
    uint32_t ioReactor = AffinityManager::Instance()->GetIOReactorCore();
    spdk_event* event = spdk_event_allocate(ioReactor, HandleSubmitCopy, nullptr, nullptr);
    spdk_event_call(event);
    while (done == false);
    EXPECT_EQ(verifyResult, true);
}

void
IoatTest::HandleSubmitCopy(void* arg1, void* arg2)
{
    IoatApi::SubmitCopy(dst, src, BUFFER_SIZE, Verify, nullptr);
}

void
IoatTest::InitializeBuffer(void)
{
    done = false;
    verifyResult = false;
    memset(src, PATTERN, BUFFER_SIZE);
    memset(ori, PATTERN, BUFFER_SIZE);
    memset(dst, 0,BUFFER_SIZE);
}

void
IoatTest::Verify(void* argument)
{
    if (0 == memcmp(ori, dst, BUFFER_SIZE))
    {
        verifyResult = true;
    }
    else
    {
        verifyResult = false;
    }

    done = true;
}

}

GTEST_API_ int main(int argc, char **argv) {
  Spdk* spdk = Spdk::Instance();
  spdk->Init(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
