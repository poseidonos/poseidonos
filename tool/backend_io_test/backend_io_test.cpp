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

#include "tool/library_unit_test/library_unit_test.h"
pos::LibraryUnitTest libraryUnitTest;

#include <atomic>
#include <cstdio>
#include <iostream>

#include "air/src/api/Air.h"
#include "mk/ibof_config.h"
#include "spdk/pos.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/bio/ubio.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/device_manager.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/array_config.h"
#include "src/include/io_error_type.h"
#include "src/include/meta_const.h"
#include "src/io/frontend_io/aio.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/main/poseidonos.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "tool/backend_io_test/io_config.h"
namespace pos
{
void FlushIO(void* mem, bool write, uint32_t lsid, CallbackSmartPtr callback);

class InternalReadDummyHandler : public Callback
{
public:
    InternalReadDummyHandler(bool isFront, void* mem,
        uint32_t lsid, IOConfig* ioConfig)
    : Callback(isFront),
      mem(mem),
      lsid(lsid),
      ioConfig(ioConfig)
    {
    }
    ~InternalReadDummyHandler(void) override{};

private:
    void* mem;
    uint32_t lsid;
    IOConfig* ioConfig;
    bool
    _DoSpecificJob(void)
    {
        CallbackSmartPtr callback(new DummyCallbackHandler(false, ioConfig));
        FlushIO(mem, true, lsid, callback);
        return true;
    }
};

class IODispatcherTest : public IOConfig
{
public:
    IODispatcherTest(uint32_t queueDepth, uint32_t blockSize, uint32_t timeInSeconds)
    : IOConfig(queueDepth, blockSize, timeInSeconds)
    {
        cpu_set_t generalCorePool = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::GENERAL_USAGE);
        sched_setaffinity(0, sizeof(generalCorePool), &generalCorePool);

        memSize = queueDepth * blockSize / ArrayConfig::SECTOR_SIZE_BYTE; // 128k
        mem = pos::Memory<ArrayConfig::SECTOR_SIZE_BYTE>::Alloc(memSize);
        IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo("POSArray");
        DeviceSet<string> name = info->GetDevNames();
        devSize = name.data.size();
        devs = DeviceManagerSingleton::Instance()->GetDevs();
        lba = 0;
        queueDepth *= devSize;
    }
    ~IODispatcherTest(void)
    {
        pos::Memory<>::Free(mem);
    }
    void
    Submit(void)
    {
        for (auto dev : devs)
        {
            if (dev->GetType() == DeviceType::SSD)
            {
                pendingIO++;
                UbioSmartPtr bio(new Ubio(mem, memSize, 0));
                bio->dir = UbioDir::Write;
                bio->SetLba(lba);
                bio->SetUblock(dev);
                CallbackSmartPtr callback(new DummyCallbackHandler(false, this));
                bio->SetCallback(callback);
                int ret = IODispatcherSingleton::Instance()->Submit(bio);
                airlog("PERF_BACKEND_TEST", "AIR_WRITE", 0, blockSize);
            }
        }
        pendingIO--;
    }

private:
    uint64_t memSize;
    uint32_t devSize;
    uint64_t lba;
    void* mem;
    std::vector<UblockSharedPtr> devs;
};

class IOSubmitHandlerTest : public IOConfig
{
public:
    IOSubmitHandlerTest(uint32_t queueDepth, uint32_t blockSize, uint32_t timeInSeconds)
    : IOConfig(queueDepth, blockSize, timeInSeconds)
    {
        cpu_set_t generalCorePool = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::GENERAL_USAGE);
        sched_setaffinity(0, sizeof(generalCorePool), &generalCorePool);
        memSize = blockSize / ArrayConfig::SECTOR_SIZE_BYTE * 32; // Max Stripe Size
        mem = pos::Memory<ArrayConfig::SECTOR_SIZE_BYTE>::Alloc(memSize);
        lsid = 0;
    }
    ~IOSubmitHandlerTest(void)
    {
        pos::Memory<>::Free(mem);
    }
    void
    Submit(void)
    {
        uint64_t blocksInStripe = 0;
        char* bufferptr = static_cast<char*>(mem);
        CallbackSmartPtr callback(new DummyCallbackHandler(false, this));
        FlushIO(mem, true, lsid, callback);
        lsid = (lsid + 1) % queueDepth;
    }

private:
    uint64_t memSize;
    uint64_t lsid;
    void* mem;
};

class IOSubmitHandlerMixedTest : public IOConfig
{
public:
    IOSubmitHandlerMixedTest(uint32_t queueDepth, uint32_t blockSize, uint32_t timeInSeconds)
    : IOConfig(queueDepth, blockSize, timeInSeconds)
    {
        cpu_set_t generalCorePool = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::GENERAL_USAGE);
        sched_setaffinity(0, sizeof(generalCorePool), &generalCorePool);
        memSize = blockSize / ArrayConfig::SECTOR_SIZE_BYTE * 32; // Max Stripe Size
        mem = pos::Memory<ArrayConfig::SECTOR_SIZE_BYTE>::Alloc(memSize);
        lsid = 0;
    }
    ~IOSubmitHandlerMixedTest(void)
    {
        pos::Memory<>::Free(mem);
    }
    void
    Submit(void)
    {
        uint64_t blocksInStripe = 0;
        char* bufferptr = static_cast<char*>(mem);
        CallbackSmartPtr callback(new InternalReadDummyHandler(false, mem, lsid, this));
        FlushIO(mem, false, lsid, callback);
        lsid = (lsid + 1) % queueDepth;
    }

private:
    uint64_t memSize;
    uint64_t lsid;
    void* mem;
};

void
FlushIO(void* mem, bool write, uint32_t lsid, CallbackSmartPtr callback)
{
    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo("POSArray");
    DeviceSet<string> name = info->GetDevNames();
    uint32_t devSize = name.data.size();
    std::list<BufferEntry> bufferList;
    bufferList.clear();

    uint64_t blocksInStripe = 0;
    char* bufferptr = static_cast<char*>(mem);
    for (uint32_t i = 0; i < devSize - 1; i++)
    {
        BufferEntry bufferEntry(bufferptr, BLOCKS_IN_CHUNK);
        bufferList.push_back(bufferEntry);
        bufferptr = bufferptr + BLOCKS_IN_CHUNK * ArrayConfig::BLOCK_SIZE_BYTE;
        blocksInStripe += BLOCKS_IN_CHUNK;
    }
    LogicalBlkAddr startLSA = {
        .stripeId = lsid,
        .offset = 0};
    IOSubmitHandlerStatus errorReturned = IIOSubmitHandler::GetInstance()->SubmitAsyncIO(
        IODirection::WRITE,
        bufferList,
        startLSA, blocksInStripe,
        USER_DATA,
        callback, "POSArray");
    if (write)
    {
        airlog("PERF_BACKEND_TEST", "AIR_WRITE", 0, blocksInStripe * ArrayConfig::BLOCK_SIZE_BYTE);
    }
    else
    {
        airlog("PERF_BACKEND_TEST", "AIR_READ", 0, blocksInStripe * ArrayConfig::BLOCK_SIZE_BYTE);
    }
}

void
test1_iodispatcher(void)
{
    IODispatcherTest ioDispatcherTest(128, 128 * 1024, 20);
    libraryUnitTest.TestStart(1);
    ioDispatcherTest.ExecuteLoop();
    libraryUnitTest.TestResult(1, true);
}

void
test2_iosubmithandler(void)
{
    IOSubmitHandlerTest ioSubmitTest(256, ArrayConfig::BLOCKS_PER_CHUNK * ArrayConfig::BLOCK_SIZE_BYTE, 20);
    libraryUnitTest.TestStart(2);
    ioSubmitTest.ExecuteLoop();
    libraryUnitTest.TestResult(2, true);
}

void
test3_iosubmithandler_mixed(void)
{
    IOSubmitHandlerMixedTest ioSubmitTest(256, ArrayConfig::BLOCKS_PER_CHUNK * ArrayConfig::BLOCK_SIZE_BYTE, 20);
    libraryUnitTest.TestStart(3);
    ioSubmitTest.ExecuteLoop();
    libraryUnitTest.TestResult(3, true);
}
} // namespace pos

int
main(int argc, char* argv[])
{
    libraryUnitTest.Initialize(argc, argv, "../../");

    pos::test1_iodispatcher();
    pos::test2_iosubmithandler();
    pos::test3_iosubmithandler_mixed();
    libraryUnitTest.SuccessAndExit();
    return 0;
}
