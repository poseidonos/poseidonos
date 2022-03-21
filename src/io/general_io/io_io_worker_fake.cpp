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

#include "src/io_scheduler/io_worker.h"

#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <random>
#include <thread>

#include "io_submit_handler_test.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/memory.h"
#include "src/io_scheduler/io_queue.h"
#include "src/bio/ubio.h"

namespace pos
{
static uint32_t timeSeed;
static uint8_t (*storage)[BLOCK_SIZE];

static void
EmulateWrite(UbioSmartPtr ubio)
{
    uint64_t blockIndex = ChangeSectorToBlock(ubio->GetLba());
    // std::cout << "Write Block #" << blockIndex << " , size: " << ubio->size <<
    //     " Bytes" << std::endl;
    memcpy(storage[blockIndex], ubio->GetBuffer(), ubio->GetSize());

    IoCompleter ioCompleter(ubio);
    ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
}

static void
EmulateRead(UbioSmartPtr ubio)
{
    uint64_t blockIndex = ChangeSectorToBlock(ubio->GetLba());
    // std::cout << "Read Block #" << blockIndex << " , size: " << ubio->size <<
    //     " Bytes" << std::endl;
    memcpy(ubio->GetBuffer(), storage[blockIndex], ubio->GetSize());

    IoCompleter ioCompleter(ubio);
    ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor 
 *
 * @Param    coreAffinityInput
 */
/* --------------------------------------------------------------------------*/
IOWorker::IOWorker(cpu_set_t cpuSetInput, uint32_t id,
    DeviceDetachTrigger* detachTriggerArg,
    QosManager* qosManagerArg, EventScheduler* eventSchedulerArg)
: cpuSet(cpuSetInput),
  ioQueue(new IOQueue),
  currentOutstandingIOCount(0),
  exit(false),
  id(id),
  detachTrigger(detachTriggerArg),
  qosManager(qosManagerArg),
  eventScheduler(eventSchedulerArg)
{
    thread = new std::thread(&IOWorker::Run, this);

    timeSeed = std::chrono::system_clock::now().time_since_epoch().count();

    uint32_t storageSize = IOSubmitHandlerTest::BUFFER_COUNT *
        IOSubmitHandlerTest::SSD_COUNT * BLOCKS_IN_CHUNK * BLOCK_SIZE;
    storage = reinterpret_cast<uint8_t(*)[BLOCK_SIZE]>(
        new uint8_t[storageSize]);

    std::cout << "Allocated Storage Size: " << storageSize << "Bytes, "
              << IOSubmitHandlerTest::BUFFER_COUNT *
            IOSubmitHandlerTest::SSD_COUNT * BLOCKS_IN_CHUNK
              << "Blocks."
              << std::endl;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
IOWorker::~IOWorker(void)
{
    delete storage;
    delete ioQueue;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Push bio for IOWorker
 *           MPSC queue (handlers -> IOWorker)
 *
 * @Param    pobjInput
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::EnqueueUbio(UbioSmartPtr ubio)
{
    ioQueue->EnqueueUbio(ubio);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Big loop of IOWorker
 *           Get bio from IOQueue and submit to device
 *           Only libaio is supported now
 */
/* --------------------------------------------------------------------------*/
void
IOWorker::Run(void)
{
    pthread_setname_np(pthread_self(), "IOWorker");
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);

    while (false == exit)
    {
        UbioSmartPtr ubio = ioQueue->DequeueUbio();
        if (nullptr != ubio)
        {
            _SubmitAsyncIO(ubio);
        }
    }
}

void
IOWorker::_SubmitAsyncIO(UbioSmartPtr ubio)
{
    switch (ubio->dir)
    {
        case UbioDir::Read:
            EmulateRead(ubio);
            break;

        case UbioDir::Write:
            EmulateWrite(ubio);
            break;

        default:
            std::cout << "Undefined Ubio Direction!!: " << std::endl;
            break;
    }
}

} // namespace pos
