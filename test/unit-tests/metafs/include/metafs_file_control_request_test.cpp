#include "src/metafs/include/metafs_control_request.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFsFileControlRequest, CheckValidity)
{
    MetaFsFileControlRequest req;

    EXPECT_EQ(req.IsValid(), false);

    req.reqType = MetaFsFileControlType::FileOpen;

    EXPECT_EQ(req.IsValid(), true);
}

} // namespace pos
