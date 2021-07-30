#include "src/metafs/mdi/mdi_manager.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaIntegrityManager, CheckDefaultMethods)
{
    MetaIntegrityManager mgr;
    MetaFsRequestBase reqMsg;

    EXPECT_EQ(mgr.CheckReqSanity(reqMsg), POS_EVENT_ID::SUCCESS);
    EXPECT_EQ(mgr.ProcessNewReq(reqMsg), POS_EVENT_ID::SUCCESS);
}

} // namespace pos
