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

#include <iostream>
#include "test/integration-tests/framework/io_generator/io_generator.h"
#include "test/integration-tests/framework/write_tester/write_tester.h"
#include "src/logger/logger.h"
#include "test/integration-tests/framework/write_tester/volume_io_fake.h"

namespace pos
{
ThreadPool::ThreadPool(int size)
: numOfThreads(size),
  stopAll(false)
{
    workerThreads.reserve(numOfThreads);

    InitThreadVector();
}

ThreadPool::~ThreadPool(void)
{
    stopAll = true;
    conditionVar.notify_all();

    WaitCompletion();
}

void
ThreadPool::InitThreadVector(void)
{
    for (int threadCount = 0; threadCount < numOfThreads; ++threadCount)
    {
        workerThreads.emplace_back([this]() 
        { 
            this->WorkerThread(); 
        });
    }
}

void
ThreadPool::WorkerThread()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(jobMutex);
        conditionVar.wait(lock, [this]() 
        { 
            return !this->jobs.empty() || stopAll; 
        });

        if ((true == stopAll) && (true == this->jobs.empty()))
        {
            return;
        }

        std::function<void()> jobToRun = std::move(jobs.front());
        jobs.pop();

        lock.unlock();

        jobToRun();
    }
}

void
ThreadPool::WaitCompletion(void)
{
    for (auto& thread : workerThreads)
    {
        thread.join();
    }
}

template <class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::EnqueueJob(F&& f, Args&&... args)
{
    if (true == stopAll)
    {
        throw std::runtime_error("Thread pool stopped");
    }

    using return_type = typename std::result_of<F(Args...)>::type;
    auto job = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> job_result_future = job->get_future();
    {
        std::lock_guard<std::mutex> lock(jobMutex);
        jobs.push([job]() { (*job)(); });
    }
    conditionVar.notify_one();

    return job_result_future;
}

IoParameter::IoParameter(void)
:  genType(IO_GEN_TYPE_SEQUENTIAL),
   blockGenType(IO_GEN_BLOCK_TYPE_FIXED),
   startOffset(0),
   blockSize(8),
   prevOffset(0),
   curOffset(0),
   generatedCount(0),
   remainCount(0),
   sync(true)
{
}

IoGenerator::IoGenerator(WriteTester* writeTester_, uint32_t numOfIothread)
: threadPool(nullptr),
  writeTester(writeTester_)
{
    Init(numOfIothread);
}

IoGenerator::~IoGenerator(void)
{
    if (nullptr != threadPool)
    {
        delete threadPool;
    }

    ::operator delete(volumeIoPool);
}

bool
IoGenerator::Init(uint32_t numOfJobs)
{
    if (nullptr == threadPool)
    {
        threadPool = new ThreadPool(numOfJobs);
    }

    std::srand(0);
    generatorBitmap.reset();

    return true;
}

void
IoGenerator::WaitCompletion(void)
{
    threadPool->WaitCompletion();
}

int genSeqWriteIo(IoGenerator* ioGen, uint32_t volId, IoParameter* param)
{
    param->generatedCount++;

    bool ret = (0 == param->remainCount) ? true : false;
    if (true == param->sync)
    {
        FakeVolumeIo* volumeIo = ioGen->GetAvailableVolumeIo();
        volumeIo->SetVolumeId(volId);
        volumeIo->SetSectorRba(param->curOffset);

        assert(nullptr != volumeIo);

        // TODO: block size
        ioGen->writeTester->WriteIo(volumeIo, 8 /* volumeIo->GetSize() / 512 */);
    }

    param->curOffset = param->prevOffset + param->blockSize;
    
    POS_TRACE_DEBUG(EID(VOLUMEIO_DEBUG_SUBMIT),
        "volumeId {}, startOffset {}, prevOffset {}, curOffset{}, generatedCount {}, remainCount {}",
        volId, param->startOffset, param->prevOffset, param->curOffset, param->generatedCount, param->remainCount);

    param->prevOffset = param->curOffset;

    return 0;
}

int genRanWriteIo(IoGenerator* ioGen, uint32_t volId, IoParameter* param)
{
    param->generatedCount++;

    bool ret = (0 == param->remainCount) ? true : false;
    if (true == param->sync)
    {
        FakeVolumeIo* volumeIo = ioGen->GetAvailableVolumeIo();
        volumeIo->SetVolumeId(volId);
        volumeIo->SetSectorRba(param->curOffset);

        assert(nullptr != volumeIo);
        
        ioGen->writeTester->WriteIo(volumeIo, 8 /* volumeIo->GetSize() / 512 */);
    }

    POS_TRACE_DEBUG(EID(VOLUMEIO_DEBUG_SUBMIT),
        "volumeId {}, startOffset {}, prevOffset {}, curOffset{}, generatedCount {}, remainCount {}",
        volId, param->startOffset, param->prevOffset, param->curOffset, param->generatedCount, param->remainCount);

    param->prevOffset = param->curOffset;

    // TODO: Generates a random offset within the rba range of the target volume.
    param->curOffset = std::rand();

    return 0;
}

void
IoGenerator::GenerateWriteIo(bool sync)
{
    uint32_t arrayId = 0;
    uint32_t volId = 0;
    while (0 < generatorBitmap.count())
    {
        if (true == generatorBitmap.test(volId))
        {
            if (IO_GEN_TYPE_RANDOM == ioParam[volId].genType)
            {
                GenerateRandomIo(sync, arrayId, volId);
            }
            else
            {
                GenerateSequentialIo(sync, arrayId, volId);
            }
        }

        if (0 >= ioParam[volId].remainCount)
        {
            generatorBitmap.set(std::size_t(volId), false);
        }
        
        volId = (++volId) % MAX_VOLUME_NUM;
    }
}

void
IoGenerator::GenerateRandomIo(bool sync, uint32_t arrayId, uint32_t volId)
{
    // TODO: Generates a random offset within the rba range of the target volume.
    ioParam[volId].curOffset = std::rand();

    ioParam[volId].sync = sync;
    ioParam[volId].remainCount--;

    futureQueue.emplace(threadPool->EnqueueJob(genRanWriteIo, this, volId, &ioParam[volId]));
}

void
IoGenerator::GenerateSequentialIo(bool sync, uint32_t arrayId, uint32_t volId)
{
    ioParam[volId].sync = sync;
    ioParam[volId].remainCount--;

    futureQueue.emplace(threadPool->EnqueueJob(genSeqWriteIo, this, volId, &ioParam[volId]));
}

bool
IoGenerator::IssueIo(void)
{
    bool ret = false;
    IoParameter p;
    uint32_t arrayId = 0;
    uint32_t volId = 0;

    bool retry = false;
    std::future<int> f;

    while (futureQueue.try_pop(f))
    {
        int ret = f.get();
        // TODO: Issue async io. 
    }
    return retry;
}

void 
IoGenerator::SetConfiguration(uint32_t volId, uint32_t startOffset, uint32_t totalCount,
    IO_GEN_TYPE genType, uint32_t blockSize)
{
    ioParam[volId].genType = genType;
    ioParam[volId].startOffset = startOffset;
    ioParam[volId].blockSize = blockSize;
    ioParam[volId].generatedCount = 0;
    ioParam[volId].remainCount = totalCount;
    ioParam[volId].prevOffset = ioParam[volId].startOffset;

    generatorBitmap.set(std::size_t(volId), true);

    volumeIoPool = static_cast<FakeVolumeIo *>(::operator new(sizeof(FakeVolumeIo) * totalCount));
    
    int arrayId = 0;
    for (int volumeIoPoolIdx = 0; volumeIoPoolIdx < totalCount; volumeIoPoolIdx++)
    {
        // TODO: allocate memory based on block size and total io count (dataBufferPool = malloc(512 * blockSize))
        dataBufferPool = nullptr;
        FakeVolumeIo* volumeIo = new (&volumeIoPool[volumeIoPoolIdx]) FakeVolumeIo(dataBufferPool, blockSize, arrayId);
        availableVolumeIoQueue.push(&volumeIoPool[volumeIoPoolIdx]);
    }
}

FakeVolumeIo*
IoGenerator::GetAvailableVolumeIo(void)
{
    FakeVolumeIo* volumeIo = availableVolumeIoQueue.front();

    if (nullptr == volumeIo)
    {
        return nullptr;
    }

    availableVolumeIoQueue.pop();
    volumeIo->SetAllocated();
    return volumeIo;
}
} // namespace pos
