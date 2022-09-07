/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/metafs/mim/metafs_io_handler_base.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include <atomic>
#include <string>
#include <thread>
#include <utility>

namespace pos
{
class MetaFsIoHandlerBaseTest : public MetaFsIoHandlerBase
{
public:
    explicit MetaFsIoHandlerBaseTest(const int threadId, const int coreId,
        const std::string& threadName)
    : MetaFsIoHandlerBase(threadId, coreId, threadName)
    {
        touched = false;
    }
    virtual ~MetaFsIoHandlerBaseTest(void)
    {
    }
    virtual void StartThread(void) override
    {
        th_ = new std::thread(std::bind(&MetaFsIoHandlerBaseTest::Execute, this));
    }
    virtual bool AddArrayInfo(const int arrayId, const MaxMetaLpnMapPerMetaStorage& map) override
    {
        return true;
    }
    virtual bool RemoveArrayInfo(const int arrayId) override
    {
        return true;
    }
    void Execute(void)
    {
        while (!threadExit_)
        {
            touched = true;
            usleep(1000);
        }
    }
    virtual bool GetTouched(void)
    {
        return touched;
    }
    virtual std::string GetThreadName(void)
    {
        return threadName_;
    }
    virtual cpu_set_t GetAffinity(void)
    {
        cpu_set_t cpus;
        CPU_ZERO(&cpus);
        CPU_SET(coreId_, &cpus);
        return cpus;
    }

private:
    std::atomic<bool> touched;
};

TEST(MetaFsIoHandlerBaseTest, testIfTheThreadCanBeStartAndStop)
{
    const int coreId = 1;
    const std::string threadName = "TestThread";
    MetaFsIoHandlerBaseTest test(0, coreId, threadName);
    test.StartThread();
    usleep(2000);
    test.ExitThread();

    EXPECT_TRUE(test.GetTouched());
}

TEST(MetaFsIoHandlerBaseTest, PrepareThread_testIfTheThreadCanBeSetThreadNameAndAffinity)
{
    const int coreId = 1;
    const std::string threadName = "TestThread";
    const std::string threadNameExpected = threadName + ":" + std::to_string(coreId) + ":0";
    MetaFsIoHandlerBaseTest test(0, coreId, threadName);

    EXPECT_EQ(test.GetThreadName(), threadNameExpected);

    cpu_set_t cpus = test.GetAffinity();
    EXPECT_TRUE(CPU_ISSET(coreId, &cpus));
}
} // namespace pos
