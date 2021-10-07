#include "src/device/unvme/unvme_device_context.h"

#include <gtest/gtest.h>

namespace pos
{

TEST(UnvmeDeviceContext, IncAdminCommandCount_testIfIncreasedSuccessfully)
{
    // Given
    UnvmeDeviceContext devCtx;

    // When
    devCtx.IncAdminCommandCount();
    bool ret = devCtx.IsAdminCommandPendingZero();

    // Then
    EXPECT_FALSE(ret);

}

TEST(UnvmeDeviceContext, DecAdminCommandCount_testIfDecreasedSuccessfully)
{
    // Given
    UnvmeDeviceContext devCtx;
    devCtx.IncAdminCommandCount();

    // When
    devCtx.DecAdminCommandCount();
    bool ret = devCtx.IsAdminCommandPendingZero();

    // Then
    EXPECT_TRUE(ret);
}

} // namespace pos
