#include "src/device/unvme/unvme_io_context.h"

#include <gtest/gtest.h>

namespace pos
{

TEST(UnvmeIOContext, SetAdminCommand_testIfSetProperly)
{
    // Given
    UnvmeIOContext ioCtx(nullptr, nullptr, 0, false);

    // When
    ioCtx.SetAdminCommand();

    // Then
    EXPECT_TRUE(ioCtx.IsAdminCommand());

}

} // namespace pos
