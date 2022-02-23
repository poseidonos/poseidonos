#include "src/metafs/mim/metafs_io_multi_q.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFsIoMultiQ, Normal)
{
    const uint32_t QUEUE_COUNT = MetaFsConfig::DEFAULT_MAX_CORE_COUNT;
    const uint32_t REQ_COUNT = 5;
    uint32_t total_count = 0;

    MetaFsIoMultiQ* multiQ = new MetaFsIoMultiQ();
    multiQ->Clear();

    // push
    for (uint32_t q = 0; q < QUEUE_COUNT; q++)
    {
        for (uint32_t req = 0; req < REQ_COUNT; req++)
        {
            MetaFsIoRequest *reqMsg = new MetaFsIoRequest();
            bool result = multiQ->EnqueueReqMsg(q, reqMsg);
            EXPECT_TRUE(result);
            total_count++;
        }
    }

    // pop
    for (uint32_t q = 0; q < QUEUE_COUNT; q++)
    {
        for (uint32_t req = 0; req < REQ_COUNT; req++)
        {
            MetaFsIoRequest *reqMsg = multiQ->DequeueReqMsg(q);
            EXPECT_NE(reqMsg, nullptr);
            total_count--;
            delete reqMsg;
        }

        bool result = multiQ->IsEmpty(q);
        EXPECT_TRUE(result);
    }

    EXPECT_EQ(total_count, 0);

    multiQ->Clear();

    delete multiQ;
}

TEST(MetaFsIoMultiQ, Q_Depth)
{
    const uint32_t REQ_COUNT = 64 * 1024 * 10;
    uint32_t total_count = 0;

    MetaFsIoMultiQ* multiQ = new MetaFsIoMultiQ();
    multiQ->Clear();

    // push
    for (uint32_t req = 0; req < REQ_COUNT; req++)
    {
        MetaFsIoRequest *reqMsg = new MetaFsIoRequest();
        bool result = multiQ->EnqueueReqMsg(0, reqMsg);
        EXPECT_TRUE(result);
        total_count++;
    }

    // pop
    for (uint32_t req = 0; req < REQ_COUNT; req++)
    {
        MetaFsIoRequest *reqMsg = multiQ->DequeueReqMsg(0);
        EXPECT_NE(reqMsg, nullptr);
        total_count--;
        delete reqMsg;
    }

    bool result = multiQ->IsEmpty(0);
    EXPECT_TRUE(result);

    EXPECT_EQ(total_count, 0);

    multiQ->Clear();

    delete multiQ;
}

} // namespace pos
