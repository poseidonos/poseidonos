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

#ifndef __MFS_UT_FRAMEWORK_H__
#define __MFS_UT_FRAMEWORK_H__

#include <iostream>
#include <string>

#include "metafs_log.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#undef Max
#include <gtest/gtest.h>

#define BEGIN_UT()                                                       \
    do                                                                   \
    {                                                                    \
        std::cout << "\033[1;32m" << "Begin TC: " << __func__ << "\033[0m" << std::endl; \
    } while (0)
#define END_UT()                                                          \
    do                                                                    \
    {                                                                     \
        std::cout << "\033[1;32m" << "TC Passed: " << __func__ << "\033[0m" << std::endl; \
    } while (0)

namespace pos
{
enum MetaFsUTFailType
{
    GenericFuncionTestFailed = 0,

    Max,
};

class MetaFsUtFailTable
{
public:
    MetaFsUTFailType type;
    std::string failMsg;
};

const MetaFsUtFailTable metaFsUtFailTable[static_cast<uint32_t>(MetaFsUTFailType::Max)] =
    {
        {GenericFuncionTestFailed, "GenericFunctionTestFailed..."}};

class MetaFsUTException : public std::exception
{
public:
    MetaFsUTException(MetaFsUTFailType type, const char* func, std::string msg = nullptr)
    {
        POS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_TEST_FAILED,
            "Unit test stopped with failure.\n{} {} {}",
            metaFsUtFailTable[type].failMsg,
            std::string(func), std::string(msg));
    }
    void
    ExitUT(void) const throw()
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Unit test program exit.");
        exit(-1);
    }
};

class MetaFsUnitTestBase
{
public:
    virtual void Setup(void) = 0;
    virtual void TearDown(void) = 0;

    virtual void
    NotifyTCFailAndExit(void)
    {
        MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_TEST_FAILED,
            "Unit test stopped with failure.");
        exit(-1);
    }
};
} // namespace pos
#endif // __MFS_UT_FRAMEWORK_H__
