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

#include <assert.h>

#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_scheduler/io_worker.h"

namespace pos
{
static uint32_t CORE_FOR_IO_WORKER = 3;

IODispatcher::IODispatcher(EventFrameworkApi* eventFrameworkApiArg,
    EventScheduler* eventSchedulerArg)
: ioWorkerCount(0),
  deviceAllocationTurn(0),
  eventScheduler(eventSchedulerArg)
{
    cpu_set_t cpuSet;

    CPU_ZERO(&cpuSet);
    CPU_SET(CORE_FOR_IO_WORKER, &cpuSet);

    IOWorker* retIOWorker = new IOWorker(cpuSet, 0);
    ioWorkerMap.insert(IOWorkerPair(CORE_FOR_IO_WORKER, retIOWorker));
}

IODispatcher::~IODispatcher(void)
{
    IOWorkerMapIter it;
    for (it = ioWorkerMap.begin(); it != ioWorkerMap.end(); it++)
    {
        delete it->second;
    }
}

} // namespace pos
