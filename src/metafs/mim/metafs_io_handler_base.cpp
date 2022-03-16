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

#include "metafs_io_handler_base.h"

#include <string>

#include "src/metafs/common/metafs_common.h"

namespace pos
{
MetaFsIoHandlerBase::MetaFsIoHandlerBase(const int threadId, const int coreId,
    const std::string& threadName)
: threadId_(threadId),
  coreId_(coreId),
  th_(nullptr),
  threadExit_(false),
  threadName_(threadName)
{
    threadName_.append(":");
    threadName_.append(std::to_string(coreId_));
    threadName_.append(":");
    threadName_.append(std::to_string(threadId_));
}

// LCOV_EXCL_START
MetaFsIoHandlerBase::~MetaFsIoHandlerBase(void)
{
    if (th_)
    {
        delete th_;
        th_ = nullptr;
    }
}
// LCOV_EXCL_STOP

void
MetaFsIoHandlerBase::ExitThread(void)
{
    threadExit_ = true;
    th_->join();
}

void
MetaFsIoHandlerBase::PrepareThread(void) const
{
    _UpdateThreadName();
    _UpdateCpuPinning();
}

void
MetaFsIoHandlerBase::_UpdateThreadName(void) const
{
    pthread_setname_np(pthread_self(), threadName_.c_str());
}

void
MetaFsIoHandlerBase::_UpdateCpuPinning(void) const
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(coreId_, &cpus);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpus);
}

std::string
MetaFsIoHandlerBase::GetLogString(void) const
{
    std::string log("threadName: ");
    log.append(threadName_);
    log.append(", threadId: " + std::to_string(threadId_));
    log.append(", coreId: " + std::to_string(coreId_));
    return log;
}
} // namespace pos
