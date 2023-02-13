/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>
#include <iostream>
#include <cstdlib>
#include <tuple>
#include <atomic>
#include <bitset>
#include <tbb/concurrent_queue.h>

namespace pos
{
class WriteTester;
class FakeVolumeIo;

class ThreadPool
{
public:
    ThreadPool(int size);
    ~ThreadPool(void);

    int GetNumOfThread(void) { return numOfThreads; }
    void WaitCompletion(void);

    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args);
    
private:
    void WorkerThread();
    void InitThreadVector(void);

    int numOfThreads;
    std::vector<std::thread> workerThreads;
    std::queue<std::function<void()>> jobs;

    std::condition_variable conditionVar;
    std::mutex jobMutex;

    bool stopAll;
};

enum IO_GEN_TYPE
{
    IO_GEN_TYPE_SEQUENTIAL = 0,
    IO_GEN_TYPE_RANDOM = 1,
};

enum IO_GEN_BLOCK_TYPE
{
    IO_GEN_BLOCK_TYPE_FIXED = 0,
    IO_GEN_BLOCK_TYPE_RANDOM = 1,
};

class IoParameter
{
public:
    IoParameter(void);
    virtual ~IoParameter(void) {};

    IO_GEN_TYPE genType;
    IO_GEN_BLOCK_TYPE blockGenType;
    uint32_t startOffset;
    uint32_t blockSize;
    uint32_t prevOffset;
    uint32_t curOffset;
    uint32_t generatedCount;
    uint32_t remainCount;
    uint32_t volId;
    bool sync;
};

class IoGenerator
{
public:
    IoGenerator(WriteTester* writeTester_, uint32_t numOfIoThread = 1);
    ~IoGenerator(void);

    bool Init(uint32_t numOfJobs);

    void SetConfiguration(uint32_t volId, uint32_t startOffset, uint32_t totalCount,
        IO_GEN_TYPE genType, uint32_t blockSize);

    FakeVolumeIo* GetAvailableVolumeIo(void);
    void GenerateRandomIo(bool sync, uint32_t arrayId, uint32_t volId);
    void GenerateSequentialIo(bool sync, uint32_t arrayId, uint32_t volId);

    bool IssueIo(void);
    void GenerateWriteIo(bool sync = false);
    void WaitCompletion(void);

    ThreadPool* threadPool;
    WriteTester* writeTester;

private:
    static const int MAX_VOLUME_NUM = 10;
    static const int MAX_IO_NUM = 512;
    std::bitset<MAX_VOLUME_NUM> generatorBitmap;
    IoParameter ioParam[MAX_VOLUME_NUM];
    tbb::concurrent_queue<std::future<int>> futureQueue;
    FakeVolumeIo* volumeIoPool;
    void* dataBufferPool;
    std::queue<FakeVolumeIo*> availableVolumeIoQueue;
};

} // namespace pos
