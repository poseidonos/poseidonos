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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "lib/spdk/include/spdk/thread.h"
#include "src/device/base/device_driver.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/spdk.h"
#include "src/include/memory.h"
#include "uram_drv.h"
#include "uram_test.h"
#include "uram_device_context.h"

namespace pos
{
static char _bdev_name[10] = "uram0";

class UramTestIOEvent
{
public:
    UramTestIOEvent(void) = delete;
    explicit UramTestIOEvent(DeviceDriver* inputDeviceDriver,
        DeviceContext* inputDeviceContext,
        UbioSmartPtr inputUbio);
    ~UramTestIOEvent(void);

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

UramTestIOEvent::UramTestIOEvent(
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

UramTestIOEvent::~UramTestIOEvent(void)
{
}

uint32_t
UramTestIOEvent::WaitOperationDone(void)
{
    while (false == operationDone)
    {
        usleep(1);
    }

    return resultValue;
}

void
UramTestIOEvent::SubmitIO(void)
{
    uint32_t completionCount = static_cast<uint32_t>(
        deviceDriver->SubmitAsyncIO(deviceContext, ubio));
    resultValue = completionCount;
    operationDone = true;
}

void
UramTestIOEvent::CheckIOCompletion(void)
{
    uint32_t completionCount = static_cast<uint32_t>(
        deviceDriver->CompleteIOs(deviceContext));
    resultValue = completionCount;
    operationDone = true;
}

UramDrvTest::UramDrvTest(DeviceDriver* inputDeviceDriver)
: NVMeTest(inputDeviceDriver)
{
    UramDeviceContext* devCtx = new UramDeviceContext(_bdev_name);
    devCtx->reactorCore = EventFrameworkApiSingleton::Instance()->GetFirstReactor();
    deviceContext = devCtx;
}
UramDrvTest::~UramDrvTest(void)
{
    if (nullptr != deviceContext)
    {
        TestClose();
        delete deviceContext;
        deviceContext = nullptr;
    }
}

void
UramDrvTest::PrintUsage(void)
{
    std::cout << "Usage: sudo uram_test" << std::endl;
}

void
UramDrvTest::_PrintDevice(void)
{
    UramDeviceContext* bdevCtx =
        static_cast<UramDeviceContext*>(deviceContext);
    string bdevName = bdevCtx->name;
    std::cout << "Bdev Name: " << bdevName << std::endl;
}

bool
UramDrvTest::_TestDeviceContextInitialized(void)
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
UramDrvTest::_SubmitIO(UbioSmartPtr ioToSubmit)
{
    UramTestIOEvent eventParam(deviceDriver, deviceContext, ioToSubmit);
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
        &_SubmitIOHandler, &eventParam, nullptr);
    spdk_event_call(event);

    uint32_t completionCount = eventParam.WaitOperationDone();
    return completionCount;
}

uint32_t
UramDrvTest::_CheckIOCompletion(void)
{
    UramTestIOEvent eventParam(deviceDriver, deviceContext, nullptr);
    struct spdk_event* event = spdk_event_allocate(
        EventFrameworkApiSingleton::Instance()->GetFirstReactor(),
        &_CheckIOCompletionHandler, &eventParam, nullptr);
    spdk_event_call(event);

    uint32_t completionCount = eventParam.WaitOperationDone();
    return completionCount;
}

void
UramDrvTest::_SubmitIOHandler(void* arg1, void* arg2)
{
    UramTestIOEvent* eventParam = static_cast<UramTestIOEvent*>(arg1);
    eventParam->SubmitIO();
}

void
UramDrvTest::_CheckIOCompletionHandler(void* arg1, void* arg2)
{
    UramTestIOEvent* eventParam = static_cast<UramTestIOEvent*>(arg1);
    eventParam->CheckIOCompletion();
}

} // namespace pos

using namespace pos;

static const char* URAM_BDEV_CREATION_RPC = "../../lib/spdk/"
                                              "scripts/rpc.py "
                                              "bdev_malloc_create -b uram0 1024 512";

DeviceDriver* deviceDriver = UramDrvSingleton::Instance();

int
main(int argc, char** argv)
{
    if ((1 == argc) || (2 == argc))
    {
        bool testSuccess = false;

        Spdk* ins = SpdkSingleton::Instance();
        ins->Init(argc, argv);

        std::cout << "---- Generating Uram Bdev Start !!!----" << std::endl;
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
        }
        std::cout << "---- Generating Uram Bdev End !!!----" << std::endl;

        std::cout << "---- Test Uram Device Driver Start !!!----" << std::endl;

        std::cout << "Test ScanDevices: " << std::endl;
        UramDrvTest deviceDriverTest(deviceDriver);
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

        std::cout << "---- Test Uram Device Driver End ----" << std::endl;
    }
    else
    {
        UramDrvTest::PrintUsage();
    }
    return 0;
}
