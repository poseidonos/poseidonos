#pragma once

#include <gtest/gtest.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace ::testing;

namespace pos
{
class SegmentCtxIntegrationTest : public ::testing::Test
{
protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

} // namespace pos
