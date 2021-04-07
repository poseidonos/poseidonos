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

#include "tool/library_unit_test/library_unit_test.h"

#include "src/io/general_io/command_timeout_handler.h"

#include <atomic>
#include <cstdio>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "spdk/nvme_spec.h"
#include "src/array/array.h"
#include "src/device/device_manager.h"
#include "src/device/spdk/nvme.hpp"
#include "src/device/unvme/unvme_drv.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/main/ibofos.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"

extern int argc;
extern char* argv;
ibofos::LibraryUnitTest libraryUnitTest;

namespace ibofos
{
std::atomic<int> testCount;
CallbackError retError;
volatile int errCount = 0;

class DummyCallbackHandler : public Callback
{
public:
    DummyCallbackHandler(bool isFront, Ubio* ubio = nullptr)
    : Callback(isFront),
      ubio(ubio)
    {
    }
    ~DummyCallbackHandler() override{};

private:
    bool completeOrigin;
    Ubio* ubio;
    bool
    _DoSpecificJob()
    {
        testCount++;
        errCount = _GetErrorCount();
        retError = _GetMostCriticalError();
        if (ubio)
        {
            ubio->CompleteOrigin();
        }
        return true;
    }
};

bool firstdev;
void
clean_test()
{
    testCount = 0;
    errCount = 0;
    firstdev = 0;
}

void
test_success(int i, bool success)
{
    if (success)
    {
        printf("### test %d is successfully done ####\n", i);
    }
    else
    {
        printf("### test %d is failed ####\n", i);
    }
}

void
test1_iotimeout_abort()
{
    clean_test();
    libraryUnitTest.TestStart(1);
    IODispatcher& ioDispatcher = *EventArgument::GetIODispatcher();

    std::vector<UBlockDevice*> devs = DeviceManagerSingleton::Instance()->GetDevs();
    struct spdk_nvme_ctrlr *ctrlr = nullptr, *prevCtrlr = nullptr;
    UBlockDevice* targetDevice = nullptr;
    UnvmeSsd* ssd = nullptr;
    struct spdk_nvme_ns* ns;
    string sn;

    for (auto& iter : devs)
    {
        if (iter->GetType() == DeviceType::SSD)
        {
            printf("%s will be timed-out \n", iter->GetName());
            ssd = dynamic_cast<UnvmeSsd*>(iter);
            ns = ssd->GetNs();
            sn = iter->GetSN();

            ctrlr = spdk_nvme_ns_get_ctrlr(ns);
            spdk_nvme_ctrlr_register_timeout_callback(ctrlr, 0,
                &Nvme::ControllerTimeoutCallback, nullptr);

            uint64_t size = 8 * 1024 / 512;

            void* mem = ibofos::Memory<512>::Alloc(size);

            UbioSmartPtr bio(new Ubio(mem, size));
            CallbackSmartPtr callback(new DummyCallbackHandler(false));
            bio->dir = UbioDir::Write;
            ArrayDevice* arrayDev = new ArrayDevice(iter, ArrayDeviceState::NORMAL);
            PhysicalBlkAddr pba = {.dev = arrayDev, .lba = 512 * 1024 * 1024 / 512};

            bio->SetPba(pba);
            bio->SetCallback(callback);
            EventArgument::GetIODispatcher()->Submit(bio);
            prevCtrlr = ctrlr;
            break;
        }
    }
    // during this time, abort command will be set.

    sleep(10);
    int testCountLocal = testCount;
    printf("callback : %d\n", testCountLocal);
    libraryUnitTest.TestResult(1, testCount == 1);
}

void
DiskIo(UBlockDevice* dev, void* ctx)
{
    struct spdk_nvme_ctrlr *ctrlr = nullptr, *prevCtrlr = nullptr;
    UBlockDevice* targetDevice = nullptr;
    UnvmeSsd* ssd = nullptr;
    struct spdk_nvme_ns* ns;
    string sn;

    if (dev->GetType() == DeviceType::SSD)
    {
        if (firstdev == true)
            return;
        firstdev = true;
        printf("%s will be timed-out \n", dev->GetName());
        ssd = dynamic_cast<UnvmeSsd*>(dev);
        ns = ssd->GetNs();
        sn = dev->GetSN();

        ctrlr = spdk_nvme_ns_get_ctrlr(ns);
        spdk_nvme_ctrlr_register_timeout_callback(ctrlr, 0,
            &Nvme::ControllerTimeoutCallback, nullptr);

        ArrayDevice* arrayDev = new ArrayDevice(dev, ArrayDeviceState::NORMAL);
        *(ArrayDevice **)ctx = arrayDev;

        for (unsigned int i = 0; i < 1024; i++)
        {
            uint64_t size = 8 * 1024 / 512;

            void* mem = ibofos::Memory<512>::Alloc(size);

            UbioSmartPtr bio(new Ubio(mem, size));
            CallbackSmartPtr callback(new DummyCallbackHandler(false));
            bio->dir = UbioDir::Write;

            PhysicalBlkAddr pba = {.dev = arrayDev, .lba = 512 * 1024 * 1024 / 512};

            bio->SetPba(pba);
            bio->SetCallback(callback);
            EventArgument::GetIODispatcher()->Submit(bio);
        }
    }
}

void
test2_io_multiple_abort()
{
    clean_test();
    libraryUnitTest.TestStart(2);
    IODispatcher& ioDispatcher = *EventArgument::GetIODispatcher();

    ArrayDevice *arrayDevice = nullptr;
    DeviceManagerSingleton::Instance()->IterateDevicesAndDoFunc(DiskIo, (void *)&arrayDevice);
    // We wait for detaching the device from the device manager.
    sleep(5);
    delete arrayDevice;
    int testCountLocal = testCount;

    printf("callback : %d\n", testCountLocal);
    libraryUnitTest.TestResult(2, testCount == 1024
        && CommandTimeoutHandlerSingleton::Instance()->IsPendingAbortZero() == true);
}

} // namespace ibofos

int
main(int argc, char* argv[])
{
    libraryUnitTest.Initialize(argc, argv);
    ibofos::test1_iotimeout_abort();
    ibofos::test2_io_multiple_abort();
    libraryUnitTest.SuccessAndExit();


    return 0;
}
