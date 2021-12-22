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

#include "unvme_test.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "src/device/base/device_driver.h"
#include "src/spdk_wrapper/nvme.hpp"
#include "src/spdk_wrapper/spdk.h"
#include "src/include/memory.h"
#include "unvme_device_context.h"
#include "unvme_drv.h"

namespace pos
{
UNVMeTest::UNVMeTest(DeviceDriver* inputDeviceDriver)
: NVMeTest(inputDeviceDriver)
{
    spdkNVMe = new Nvme("spdk_daemon");
    std::list<NsEntry*>* nsList = spdkNVMe->InitController();
    NsEntry* nsEntry = nsList->front();
    deviceContext = new UnvmeDeviceContext(nsEntry->u.nvme.ns);
}

UNVMeTest::~UNVMeTest(void)
{
    if (nullptr != deviceContext)
    {
        TestClose();
        delete deviceContext;
        deviceContext = nullptr;
    }

    if (nullptr != spdkNVMe)
    {
        delete spdkNVMe;
    }
}

void
UNVMeTest::PrintUsage(void)
{
    std::cout << "Usage: sudo unvmeTest" << std::endl;
}

void
UNVMeTest::_PrintDevice(void)
{
    UnvmeDeviceContext* devCxt =
        static_cast<UnvmeDeviceContext*>(deviceContext);

    uint32_t namespaceID = spdk_nvme_ns_get_id(devCxt->ns);
    std::cout << "Namespace ID: " << namespaceID << std::endl;
}

bool
UNVMeTest::_TestDeviceContextInitialized(void)
{
    bool testSuccessful = false;

    UnvmeDeviceContext* devCxt =
        static_cast<UnvmeDeviceContext*>(deviceContext);
    do
    {
        if (nullptr == devCxt->ns)
        {
            break;
        }

        if (nullptr != devCxt->ioQPair)
        {
            break;
        }

        testSuccessful = true;
    } while (false);

    return testSuccessful;
}
} // namespace pos

using namespace pos;

int
main(int argc, char** argv)
{
    if (1 == argc)
    {
        bool testSuccessful = false;

        srand(time(nullptr));

        SpdkSingleton::Instance()->Init(argc, argv);

        std::cout << "---- Test SPDK Device Driver Start ----" << std::endl;
        {
            DeviceDriver* deviceDriver = UnvmeDrvSingleton::Instance();
            UNVMeTest deviceDriverTest(deviceDriver);

            std::cout << "Test ScanDevices: " << std::endl;
            deviceDriverTest.TestScanDevs();

            std::cout << "Test OpenClose: " << std::endl;
            testSuccessful = deviceDriverTest.TestOpenClose();
            if (false == testSuccessful)
            {
                std::cout << "Test OpenClose Failed: " << std::endl;
            }

            // std::cout << "Test SyncIO: " << std::endl;
            // deviceDriverTest.TestOpen();
            // testSuccessful = deviceDriverTest.TestSyncIO();
            // if (false == testSuccessful)
            // {
            //     std::cout << "Test SyncIO Failed: " << std::endl;
            // }
            // deviceDriverTest.TestClose();

            std::cout << "Test SubmitAsyncIO: " << std::endl;
            deviceDriverTest.TestOpen();
            testSuccessful = deviceDriverTest.TestAsyncIO();
            if (false == testSuccessful)
            {
                std::cout << "Test SubmitAsyncIO Failed: " << std::endl;
            }
            deviceDriverTest.TestClose();
        }
        std::cout << "---- Test SPDK Device Driver End ----" << std::endl;
    }
    else
    {
        UNVMeTest::PrintUsage();
    }

    return 0;
}
