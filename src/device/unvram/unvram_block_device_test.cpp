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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "unvram_block_device_test.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "lib/spdk-19.10/include/spdk/thread.h"
#include "src/device/event_framework_api.h"
#include "src/device/spdk/spdk.hpp"
#include "src/include/memory.h"
#include "src/scheduler/callback.h"
#include "unvram_drv.h"

namespace ibofos
{
class UnvramBlockTestIOEvent
{
public:
    UnvramBlockTestIOEvent(void) = delete;
    explicit UnvramBlockTestIOEvent(UbioSmartPtr inputUbio);
    ~UnvramBlockTestIOEvent(void);

    void SubmitIO(void);
    void CheckIOCompletion(void);
    uint32_t WaitOperationDone(void);
    void AddCompletionCount(void);

private:
    UbioSmartPtr ubio;
    std::atomic<bool> operationDone;
    std::atomic<uint32_t> completionCount;
};

class UnvramBlockTestCompletion : public Callback
{
public:
    UnvramBlockTestCompletion(UnvramBlockTestIOEvent* inputEvent,
        UbioSmartPtr ubio);
    ~UnvramBlockTestCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;
    UnvramBlockTestIOEvent* ioEvent;
    UbioSmartPtr ubio;
};

UnvramBlockTestIOEvent::UnvramBlockTestIOEvent(UbioSmartPtr inputUbio)
: ubio(inputUbio),
  operationDone(false),
  completionCount(0)
{
}

UnvramBlockTestIOEvent::~UnvramBlockTestIOEvent(void)
{
}

uint32_t
UnvramBlockTestIOEvent::WaitOperationDone(void)
{
    while (false == operationDone)
    {
        usleep(1);
    }

    operationDone = false;

    return completionCount;
}

void
UnvramBlockTestIOEvent::SubmitIO(void)
{
    CallbackSmartPtr callback(new UnvramBlockTestCompletion(this, ubio));
    ubio->SetCallback(callback);
    ubio->GetUBlock()->SubmitAsyncIO(ubio);
    operationDone = true;
}

void
UnvramBlockTestIOEvent::CheckIOCompletion(void)
{
    ubio->GetUBlock()->CompleteIOs();
    operationDone = true;
}

void
UnvramBlockTestIOEvent::AddCompletionCount(void)
{
    completionCount++;
}

UnvramBlockTestCompletion::UnvramBlockTestCompletion(
    UnvramBlockTestIOEvent* inputEvent, UbioSmartPtr ubio)
: Callback(false),
  ioEvent(inputEvent),
  ubio(ubio)
{
}

bool
UnvramBlockTestCompletion::_DoSpecificJob(void)
{
    ioEvent->AddCompletionCount();

    return true;
}

UnvramBlockDeviceTest::UnvramBlockDeviceTest(DeviceDriver* inputDeviceDriver,
    uint32_t ioContextCount)
: UBlockDeviceTest(nullptr, inputDeviceDriver, ioContextCount),
  unvramBlockTestIOEvent(new UnvramBlockTestIOEvent*[ioContextCount])
{
    for (uint32_t index = 0; index < ioContextCount; index++)
    {
        unvramBlockTestIOEvent[index] = nullptr;
    }
}

UnvramBlockDeviceTest::~UnvramBlockDeviceTest(void)
{
}

void
UnvramBlockDeviceTest::PrintUsage(void)
{
    std::cout << "Usage: sudo unvram_block_device_test" << std::endl;
}

uint32_t
UnvramBlockDeviceTest::_SubmitIO(UbioSmartPtr ioToSubmit, uint32_t threadIndex)
{
    UnvramBlockTestIOEvent* eventParam = new UnvramBlockTestIOEvent(ioToSubmit);
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApi::GetFirstReactor(),
        &_SubmitIOHandler, eventParam, nullptr);
    spdk_event_call(event);

    if (nullptr != unvramBlockTestIOEvent[threadIndex])
    {
        delete unvramBlockTestIOEvent[threadIndex];
    }

    unvramBlockTestIOEvent[threadIndex] = eventParam;
    uint32_t completionCount = eventParam->WaitOperationDone();
    return completionCount;
}

uint32_t
UnvramBlockDeviceTest::_CheckIOCompletion(UBlockDevice* dev, uint32_t threadIndex)
{
    UnvramBlockTestIOEvent* eventParam = unvramBlockTestIOEvent[threadIndex];
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApi::GetFirstReactor(),
        &_CheckIOCompletionHandler, eventParam, nullptr);
    spdk_event_call(event);

    uint32_t completionCount = eventParam->WaitOperationDone();
    return completionCount;
}

void
UnvramBlockDeviceTest::_SubmitIOHandler(void* arg1, void* arg2)
{
    UnvramBlockTestIOEvent* eventParam =
        static_cast<UnvramBlockTestIOEvent*>(arg1);
    eventParam->SubmitIO();
}

void
UnvramBlockDeviceTest::_CheckIOCompletionHandler(void* arg1, void* arg2)
{
    UnvramBlockTestIOEvent* eventParam =
        static_cast<UnvramBlockTestIOEvent*>(arg1);
    eventParam->CheckIOCompletion();
}

} // namespace ibofos

using namespace ibofos;

static const char* UNVRAM_BDEV_CREATION_RPC = "../../lib/spdk-19.10/"
                                              "scripts/rpc.py "
                                              "bdev_malloc_create -b uram0 1024 512";

static void
RegisterReactorThreadHandler(void* arg1, void* arg2)
{
    UnvramBlockDeviceTest* uBlockDeviceTest =
        static_cast<UnvramBlockDeviceTest*>(arg1);
    uBlockDeviceTest->RegisterThread();
}

static void
UnregisterReactorThreadHandler(void* arg1, void* arg2)
{
    UnvramBlockDeviceTest* uBlockDeviceTest =
        static_cast<UnvramBlockDeviceTest*>(arg1);
    uBlockDeviceTest->UnregisterThread();
}

int
main(int argc, char** argv)
{
    if ((1 == argc) || (2 == argc))
    {
        bool testSuccess = false;
        uint32_t ioContextCount = 1;
        uint32_t paramsToSkip = 0;

        if (2 == argc)
        {
            ioContextCount = strtoul(argv[1], nullptr, 0);
            paramsToSkip = 1;
        }

        Spdk* ins = SpdkSingleton::Instance();
        ins->Init(argc - paramsToSkip, argv);

        std::cout << "---- Generating Unvram Bdev Start !!!----" << std::endl;
        do
        {
            FILE* fd = popen(UNVRAM_BDEV_CREATION_RPC, "r");
            if (nullptr != fd)
            {
                char output[256] = {
                    0,
                };
                if (nullptr != fgets(output, sizeof(output), fd))
                {
                    std::cout << "Unvram Bdev Generated: " << output;
                }

                pclose(fd);
            }
        } while (false);
        std::cout << "---- Generating Unvram Bdev End !!!----" << std::endl;

        std::cout << "---- Test Unvram UBlockDevice Start !!!----" << std::endl;
        {
            DeviceDriver* deviceDriver = UnvramDrvSingleton::Instance();
            UnvramBlockDeviceTest uBlockDeviceTest(deviceDriver, ioContextCount);

            std::cout << "Test ScanDevices: " << std::endl;
            testSuccess = uBlockDeviceTest.TestScanDevs();
            if (false == testSuccess)
            {
                std::cout << "Test ScanDevices Failed: " << std::endl;
            }

            uBlockDeviceTest.PrepareThreadRegister();
            struct spdk_event* e1 = spdk_event_allocate(
                EventFrameworkApi::GetFirstReactor(),
                &RegisterReactorThreadHandler, &uBlockDeviceTest,
                nullptr);
            spdk_event_call(e1);
            uBlockDeviceTest.WaitThreadRegisterDone();

            std::cout << "Test Open/Close Devices: " << std::endl;
            testSuccess = uBlockDeviceTest.TestOpenClose();
            if (false == testSuccess)
            {
                std::cout << "Test Open/Close Failed" << std::endl;
            }

            std::cout << "Test SubmitAsyncIO: " << std::endl;
            testSuccess = uBlockDeviceTest.TestAsyncIO();
            if (false == testSuccess)
            {
                std::cout << "Test SubmitAsyncIO Failed: " << std::endl;
            }

            uBlockDeviceTest.PrepareThreadUnregister();
            e1 = spdk_event_allocate(
                EventFrameworkApi::GetFirstReactor(),
                &UnregisterReactorThreadHandler, &uBlockDeviceTest,
                nullptr);
            spdk_event_call(e1);
            uBlockDeviceTest.WaitThreadUnregisterDone();
        }
        std::cout << "---- Test Unvram UBlockDevice End ----" << std::endl;
    }
    else
    {
        UnvramBlockDeviceTest::PrintUsage();
    }

    return 0;
}
