#include "src/metafs/msc/msc_req.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFsControlReqMsg, CreateMSg)
{
    MetaFsControlReqMsg msg;

    EXPECT_EQ(msg.reqType, MetaFsControlReqType::Max);
    EXPECT_EQ(msg.arrayId, INT32_MAX);
}

TEST(MetaFsControlReqMsg, CheckValidity)
{
    MetaFsControlReqMsg msg;
    EXPECT_TRUE(msg.IsValid());
}

} // namespace pos
