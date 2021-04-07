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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "lib/spdk-19.10/include/spdk/thread.h"
#include "src/device/device_driver.h"
#include "src/device/event_framework_api.h"
#include "src/device/spdk/spdk.hpp"
#include "src/include/memory.h"
#include "unvram_drv.h"
#include "unvram_test.h"
#include "uram_device_context.h"

namespace ibofos
{
static char _bdev_name[10] = "uram0";

class UnvramTestIOEvent
{
public:
    UnvramTestIOEvent(void) = delete;
    explicit UnvramTestIOEvent(DeviceDriver* inputDeviceDriver,
        DeviceContext* inputDeviceContext,
        UbioSmartPtr inputUbio);
    ~UnvramTestIOEvent(void);

    void SubmitIO(void);
    void CheckIOCompletion(void);
    uint32_t WaitOperationDone(void);

private:
    DeviceDriver* deviceDriver;
    DeviceContext* deviceContext;
    UbioSmartPtr ubio;
    std::atomic<bool> operationDone;
    std::atomic<uint32_t> resultValue;
};

UnvramTestIOEvent::UnvramTestIOEvent(
    DeviceDriver* inputDeviceDriver,
    DeviceContext* inputDeviceContext,
    UbioSmartPtr inputUbio)
: deviceDriver(inputDeviceDriver),
  deviceContext(inputDeviceContext),
  ubio(inputUbio),
  operationDone(false),
  resultValue(0)
{
}

UnvramTestIOEvent::~UnvramTestIOEvent(void)
{
}

uint32_t
UnvramTestIOEvent::WaitOperationDone(void)
{
    while (false == operationDone)
    {
        usleep(1);
    }

    return resultValue;
}

void
UnvramTestIOEvent::SubmitIO(void)
{
    uint32_t completionCount = static_cast<uint32_t>(
        deviceDriver->SubmitAsyncIO(deviceContext, ubio));
    resultValue = completionCount;
    operationDone = true;
}

void
UnvramTestIOEvent::CheckIOCompletion(void)
{
    uint32_t completionCount = static_cast<uint32_t>(
        deviceDriver->CompleteIOs(deviceContext));
    resultValue = completionCount;
    operationDone = true;
}

UnvramDrvTest::UnvramDrvTest(DeviceDriver* inputDeviceDriver)
: NVMeTest(inputDeviceDriver)
{
    UramDeviceContext* devCtx = new UramDeviceContext(_bdev_name);
    devCtx->reactorCore = EventFrameworkApi::GetFirstReactor();
    deviceContext = devCtx;
}
UnvramDrvTest::~UnvramDrvTest(void)
{
    if (nullptr != deviceContext)
    {
        TestClose();
        delete deviceContext;
        deviceContext = nullptr;
    }
}

void
UnvramDrvTest::PrintUsage(void)
{
    std::cout << "Usage: sudo unvram_test" << std::endl;
}

void
UnvramDrvTest::_PrintDevice(void)
{
    UramDeviceContext* bdevCtx =
        static_cast<UramDeviceContext*>(deviceContext);
    string bdevName = bdevCtx->name;
    std::cout << "Bdev Name: " << bdevName << std::endl;
}

bool
UnvramDrvTest::_TestDeviceContextInitialized(void)
{
    bool testSuccessful = false;
    UramDeviceContext* bdevCtx =
        static_cast<UramDeviceContext*>(deviceContext);
    do
    {
        if (nullptr == bdevCtx->name)
        {
            break;
        }
        if (0 != bdevCtx->ioCompleCnt)
        {
            break;
        }

        testSuccessful = true;
    } while (false);
    return testSuccessful;
}

uint32_t
UnvramDrvTest::_SubmitIO(UbioSmartPtr ioToSubmit)
{
    UnvramTestIOEvent eventParam(deviceDriver, deviceContext, ioToSubmit);
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApi::GetFirstReactor(),
        &_SubmitIOHandler, &eventParam, nullptr);
    spdk_event_call(event);

    uint32_t completionCount = eventParam.WaitOperationDone();
    return completionCount;
}

uint32_t
UnvramDrvTest::_CheckIOCompletion(void)
{
    UnvramTestIOEvent eventParam(deviceDriver, deviceContext, nullptr);
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApi::GetFirstReactor(),
        &_CheckIOCompletionHandler, &eventParam, nullptr);
    spdk_event_call(event);

    uint32_t completionCount = eventParam.WaitOperationDone();
    return completionCount;
}

void
UnvramDrvTest::_SubmitIOHandler(void* arg1, void* arg2)
{
    UnvramTestIOEvent* eventParam = static_cast<UnvramTestIOEvent*>(arg1);
    eventParam->SubmitIO();
}

void
UnvramDrvTest::_CheckIOCompletionHandler(void* arg1, void* arg2)
{
    UnvramTestIOEvent* eventParam = static_cast<UnvramTestIOEvent*>(arg1);
    eventParam->CheckIOCompletion();
}

} // namespace ibofos

using namespace ibofos;

static const char* UNVRAM_BDEV_CREATION_RPC = "../../lib/spdk-19.10/"
                                              "scripts/rpc.py "
                                              "bdev_malloc_create -b uram0 1024 512";

DeviceDriver* deviceDriver = UnvramDrvSingleton::Instance();

int
main(int argc, char** argv)
{
    if ((1 == argc) || (2 == argc))
    {
        bool testSuccess = false;

        Spdk* ins = SpdkSingleton::Instance();
        ins->Init(argc, argv);

        std::cout << "---- Generating Unvram Bdev Start !!!----" << std::endl;
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
        }
        std::cout << "---- Generating Unvram Bdev End !!!----" << std::endl;

        std::cout << "---- Test Unvram Device Driver Start !!!----" << std::endl;

        std::cout << "Test ScanDevices: " << std::endl;
        UnvramDrvTest deviceDriverTest(deviceDriver);
        deviceDriverTest.TestScanDevs();

        std::cout << "Test Open/Close Devices: " << std::endl;
        testSuccess = deviceDriverTest.TestOpenClose();
        if (false == testSuccess)
        {
            std::cout << "Test Open/Close Failed" << std::endl;
        }

        std::cout << "Test SubmitAsyncIO: " << std::endl;
        deviceDriverTest.TestOpen();
        testSuccess = deviceDriverTest.TestAsyncIO();
        if (false == testSuccess)
        {
            std::cout << "Test SubmitAsyncIO Failed: " << std::endl;
        }

        deviceDriverTest.TestClose();

        std::cout << "---- Test Unvram Device Driver End ----" << std::endl;
    }
    else
    {
        UnvramDrvTest::PrintUsage();
    }
    return 0;
}
