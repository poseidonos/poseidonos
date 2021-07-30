#include "src/metafs/include/metafs_aiocb_cxt.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFsAioCbCxt, CheckTagId)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetTagId(), 0);

    cb.SetTagId(10);

    EXPECT_EQ(cb.GetTagId(), 10);
}

TEST(MetaFsAioCbCxt, CheckError_Positive)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.CheckIOError(), true);

    MfsError err = { 0, false };
    cb.SetErrorStatus(err);

    EXPECT_EQ(cb.CheckIOError(), false);
}

TEST(MetaFsAioCbCxt, CheckError_Negative0)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.CheckIOError(), true);

    MfsError err = { 1, false };
    cb.SetErrorStatus(err);

    EXPECT_EQ(cb.CheckIOError(), true);
}

TEST(MetaFsAioCbCxt, CheckError_Negative1)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.CheckIOError(), true);

    MfsError err = { 0, true };
    cb.SetErrorStatus(err);

    EXPECT_EQ(cb.CheckIOError(), true);
}

TEST(MetaFsAioCbCxt, CheckCallback)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    cb.SetCallbackCount(10);
    cb.InvokeCallback();
}

TEST(MetaFsAioCbCxt, CheckOpcode)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetOpCode(), MetaFsIoOpcode::Read);
}

TEST(MetaFsAioCbCxt, CheckFd)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetFD(), 0);
}

TEST(MetaFsAioCbCxt, CheckOffset)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetOffset(), 0);
}

TEST(MetaFsAioCbCxt, CheckSize)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_EQ(cb.GetByteSize(), 0);
}

TEST(MetaFsAioCbCxt, CheckBuffer)
{
    MetaFsAioCbCxt cb(MetaFsIoOpcode::Read, 0, 0, nullptr, nullptr);

    EXPECT_TRUE(cb.GetBuffer() == nullptr);
}
} // namespace pos
