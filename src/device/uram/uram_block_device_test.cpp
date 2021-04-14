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

#include "uram_block_device_test.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "lib/spdk/include/spdk/thread.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/include/memory.h"
#include "src/event_scheduler/callback.h"
#include "uram_drv.h"

namespace pos
{
class UramBlockTestIOEvent
{
public:
    UramBlockTestIOEvent(void) = delete;
    explicit UramBlockTestIOEvent(UbioSmartPtr inputUbio);
    ~UramBlockTestIOEvent(void);

    void SubmitIO(void);
    void CheckIOCompletion(void);
    uint32_t WaitOperationDone(void);
    void AddCompletionCount(void);

private:
    UbioSmartPtr ubio;
    std::atomic<bool> operationDone;
    std::atomic<uint32_t> completionCount;
};

class UramBlockTestCompletion : public Callback
{
public:
    UramBlockTestCompletion(UramBlockTestIOEvent* inputEvent,
        UbioSmartPtr ubio);
    ~UramBlockTestCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;
    UramBlockTestIOEvent* ioEvent;
    UbioSmartPtr ubio;
};

UramBlockTestIOEvent::UramBlockTestIOEvent(UbioSmartPtr inputUbio)
: ubio(inputUbio),
  operationDone(false),
  completionCount(0)
{
}

UramBlockTestIOEvent::~UramBlockTestIOEvent(void)
{
}

uint32_t
UramBlockTestIOEvent::WaitOperationDone(void)
{
    while (false == operationDone)
    {
        usleep(1);
    }

    operationDone = false;

    return completionCount;
}

void
UramBlockTestIOEvent::SubmitIO(void)
{
    CallbackSmartPtr callback(new UramBlockTestCompletion(this, ubio));
    ubio->SetCallback(callback);
    ubio->GetUBlock()->SubmitAsyncIO(ubio);
    operationDone = true;
}

void
UramBlockTestIOEvent::CheckIOCompletion(void)
{
    ubio->GetUBlock()->CompleteIOs();
    operationDone = true;
}

void
UramBlockTestIOEvent::AddCompletionCount(void)
{
    completionCount++;
}

UramBlockTestCompletion::UramBlockTestCompletion(
    UramBlockTestIOEvent* inputEvent, UbioSmartPtr ubio)
: Callback(false),
  ioEvent(inputEvent),
  ubio(ubio)
{
}

bool
UramBlockTestCompletion::_DoSpecificJob(void)
{
    ioEvent->AddCompletionCount();

    return true;
}

UramBlockDeviceTest::UramBlockDeviceTest(DeviceDriver* inputDeviceDriver,
    uint32_t ioContextCount)
: UBlockDeviceTest(nullptr, inputDeviceDriver, ioContextCount),
  uramBlockTestIOEvent(new UramBlockTestIOEvent*[ioContextCount])
{
    for (uint32_t index = 0; index < ioContextCount; index++)
    {
        uramBlockTestIOEvent[index] = nullptr;
    }
}

UramBlockDeviceTest::~UramBlockDeviceTest(void)
{
}

void
UramBlockDeviceTest::PrintUsage(void)
{
    std::cout << "Usage: sudo uram_block_device_test" << std::endl;
}

uint32_t
UramBlockDeviceTest::_SubmitIO(UbioSmartPtr ioToSubmit, uint32_t threadIndex)
{
    UramBlockTestIOEvent* eventParam = new UramBlockTestIOEvent(ioToSubmit);
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApi::GetFirstReactor(),
        &_SubmitIOHandler, eventParam, nullptr);
    spdk_event_call(event);

    if (nullptr != uramBlockTestIOEvent[threadIndex])
    {
        delete uramBlockTestIOEvent[threadIndex];
    }

    uramBlockTestIOEvent[threadIndex] = eventParam;
    uint32_t completionCount = eventParam->WaitOperationDone();
    return completionCount;
}

uint32_t
UramBlockDeviceTest::_CheckIOCompletion(UblockSharedPtr dev, uint32_t threadIndex)
{
    UramBlockTestIOEvent* eventParam = uramBlockTestIOEvent[threadIndex];
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApi::GetFirstReactor(),
        &_CheckIOCompletionHandler, eventParam, nullptr);
    spdk_event_call(event);

    uint32_t completionCount = eventParam->WaitOperationDone();
    return completionCount;
}

void
UramBlockDeviceTest::_SubmitIOHandler(void* arg1, void* arg2)
{
    UramBlockTestIOEvent* eventParam =
        static_cast<UramBlockTestIOEvent*>(arg1);
    eventParam->SubmitIO();
}

void
UramBlockDeviceTest::_CheckIOCompletionHandler(void* arg1, void* arg2)
{
    UramBlockTestIOEvent* eventParam =
        static_cast<UramBlockTestIOEvent*>(arg1);
    eventParam->CheckIOCompletion();
}

} // namespace pos

using namespace pos;

static const char* URAM_BDEV_CREATION_RPC = "../../lib/spdk/"
                                              "scripts/rpc.py "
                                              "bdev_malloc_create -b uram0 1024 512";

static void
RegisterReactorThreadHandler(void* arg1, void* arg2)
{
    UramBlockDeviceTest* uBlockDeviceTest =
        static_cast<UramBlockDeviceTest*>(arg1);
    uBlockDeviceTest->RegisterThread();
}

static void
UnregisterReactorThreadHandler(void* arg1, void* arg2)
{
    UramBlockDeviceTest* uBlockDeviceTest =
        static_cast<UramBlockDeviceTest*>(arg1);
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

        std::cout << "---- Generating Uram Bdev Start !!!----" << std::endl;
        do
        {
            FILE* fd = popen(URAM_BDEV_CREATION_RPC, "r");
            if (nullptr != fd)
            {
                char output[256] = {
                    0,
                };
                if (nullptr != fgets(output, sizeof(output), fd))
                {
                    std::cout << "Uram Bdev Generated: " << output;
                }

                pclose(fd);
            }
        } while (false);
        std::cout << "---- Generating Uram Bdev End !!!----" << std::endl;

        std::cout << "---- Test Uram UBlockDevice Start !!!----" << std::endl;
        {
            DeviceDriver* deviceDriver = UramDrvSingleton::Instance();
            UramBlockDeviceTest uBlockDeviceTest(deviceDriver, ioContextCount);

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
        std::cout << "---- Test Uram UBlockDevice End ----" << std::endl;
    }
    else
    {
        UramBlockDeviceTest::PrintUsage();
    }

    return 0;
}
