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

#include "mfs_io_handler_base.h"
#include "metafs_common.h"

namespace pos
{
MetaFsIoHandlerBase::MetaFsIoHandlerBase(int threadId, int coreId)
: threadId(threadId),
  coreId(coreId),
  th(nullptr),
  threadExit(false),
  threadName(nullptr)
{
}

// LCOV_EXCL_START
MetaFsIoHandlerBase::~MetaFsIoHandlerBase(void)
{
    if (threadName)
    {
        delete threadName;
    }
    if (th)
    {
        delete th;
    }
}
// LCOV_EXCL_STOP

void
MetaFsIoHandlerBase::ExitThread(void)
{
    threadExit = true;
    th->join();
}

void
MetaFsIoHandlerBase::PrepareThread(const char* name)
{
    threadName = new std::string(name);
    threadName->append(std::to_string(coreId));
    threadName->append(":");
    threadName->append(std::to_string(threadId));

    _UpdateThreadName(threadName);
    _UpdateThreadCPUAffinity(coreId);
}

void
MetaFsIoHandlerBase::_UpdateThreadName(std::string* name)
{
    pthread_setname_np(pthread_self(), name->c_str());
}

void
MetaFsIoHandlerBase::_UpdateThreadCPUAffinity(uint32_t coreId)
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(coreId, &cpus);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpus);
}
} // namespace pos
