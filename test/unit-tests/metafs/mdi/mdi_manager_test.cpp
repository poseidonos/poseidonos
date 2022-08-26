#include "src/metafs/mdi/mdi_manager.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaIntegrityManager, CheckDefaultMethods)
{
    MetaIntegrityManager mgr;
    MetaFsRequestBase reqMsg;

    EXPECT_EQ(mgr.CheckReqSanity(reqMsg), EID(SUCCESS));
    EXPECT_EQ(mgr.ProcessNewReq(reqMsg), EID(SUCCESS));
}

} // namespace pos
