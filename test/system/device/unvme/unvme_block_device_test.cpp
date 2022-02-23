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

#include "unvme_block_device_test.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "src/spdk_wrapper/spdk.h"
#include "src/include/memory.h"
#include "unvme_drv.h"

namespace pos
{
UnvmeBlockDeviceTest::UnvmeBlockDeviceTest(DeviceDriver* inputDeviceDriver,
    uint32_t ioContextCount)
: UBlockDeviceTest(nullptr, inputDeviceDriver, ioContextCount)
{
    spdkNVMe = new Nvme("spdk_daemon");
    spdkNVMe->InitController();
}

UnvmeBlockDeviceTest::~UnvmeBlockDeviceTest(void)
{
    delete spdkNVMe;
}

void
UnvmeBlockDeviceTest::PrintUsage(void)
{
    std::cout << "Usage: sudo unvme_block_device_test" << std::endl;
}

} // namespace pos

using namespace pos;

int
main(int argc, char** argv)
{
    if ((1 == argc) || (2 == argc))
    {
        bool testSuccessful = false;
        uint32_t ioContextCount = 1;
        uint32_t paramsToSkip = 0;

        if (2 == argc)
        {
            ioContextCount = strtoul(argv[1], nullptr, 0);
            paramsToSkip = 1;
        }

        srand(time(nullptr));

        SpdkSingleton::Instance()->Init(argc - paramsToSkip, argv);

        std::cout << "---- Test UBlockDevice with SPDK Device Driver Start ----"
                  << std::endl;
        {
            DeviceDriver* deviceDriver = UnvmeDrvSingleton::Instance();
            UnvmeBlockDeviceTest ublockDeviceTest(deviceDriver, ioContextCount);

            std::cout << "Test ScanDevices: " << std::endl;
            testSuccessful = ublockDeviceTest.TestScanDevs();
            if (false == testSuccessful)
            {
                std::cout << "Test ScanDevices Failed: " << std::endl;
            }

            std::cout << "Test OpenClose: " << std::endl;
            testSuccessful = ublockDeviceTest.TestOpenClose();
            if (false == testSuccessful)
            {
                std::cout << "Test OpenClose Failed: " << std::endl;
            }

            // std::cout << "Test SyncIO: " << std::endl;
            // testSuccessful = ublockDeviceTest.TestSyncIO();
            // if (false == testSuccessful)
            // {
            //     std::cout << "Test SyncIO Failed: " << std::endl;
            // }

            std::cout << "Test SubmitAsyncIO: " << std::endl;
            testSuccessful = ublockDeviceTest.TestAsyncIO();
            if (false == testSuccessful)
            {
                std::cout << "Test SubmitAsyncIO Failed: " << std::endl;
            }
        }
        std::cout << "---- Test UBlockDevice with SPDK Device Driver End ----"
                  << std::endl;
    }
    else
    {
        UnvmeBlockDeviceTest::PrintUsage();
    }

    return 0;
}
