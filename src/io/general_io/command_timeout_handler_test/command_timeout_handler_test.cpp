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

#define UNVME_BUILD

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
#include "src/spdk_wrapper/nvme.hpp"
#include "src/device/unvme/unvme_drv.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/main/poseidonos.h"
#include "src/event_scheduler/io_completer.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "tool/library_unit_test/library_unit_test.h"

pos::LibraryUnitTest libraryUnitTest;
namespace pos
{
std::atomic<int> testCount;
IOErrorType retError;
volatile int errCount = 0;

class DummyCallbackHandler : public Callback
{
public:
    DummyCallbackHandler(bool isFront, UbioSmartPtr ubio = nullptr)
    : Callback(isFront),
      ubio(ubio)
    {
    }
    ~DummyCallbackHandler(void) override{};

private:
    bool completeOrigin;
    UbioSmartPtr ubio;
    bool
    _DoSpecificJob(void)
    {
        testCount++;
        errCount = _GetErrorCount();
        retError = _GetMostCriticalError();
        if (ubio)
        {
            IoCompleter ioCompleter(ubio);
            ioCompleter.CompleteOriginUbio();
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
    IODispatcher& ioDispatcher = *IODispatcherSingleton::Instance();

    std::vector<UblockSharedPtr> devs = DeviceManagerSingleton::Instance()->GetDevs();
    struct spdk_nvme_ctrlr *ctrlr = nullptr, *prevCtrlr = nullptr;
    UblockSharedPtr targetDevice = nullptr;
    UnvmeSsd* ssd = nullptr;
    struct spdk_nvme_ns* ns;
    string sn;
    for (auto& iter : devs)
    {
        if (iter->GetType() == DeviceType::SSD)
        {
            printf("%s will be timed-out \n", iter->GetName());
            ssd = dynamic_cast<UnvmeSsd*>(iter.get());
            ns = ssd->GetNs();
            sn = iter->GetSN();

            ctrlr = spdk_nvme_ns_get_ctrlr(ns);
            spdk_nvme_ctrlr_register_timeout_callback(ctrlr, 1, 1,
                &Nvme::ControllerTimeoutCallback, nullptr);

            uint64_t size = 8 * 1024 / 512;

            void* mem = pos::Memory<512>::Alloc(size);

            UbioSmartPtr bio(new Ubio(mem, size, 0));
            CallbackSmartPtr callback(new DummyCallbackHandler(false));
            bio->dir = UbioDir::Write;

            bio->SetLba(512 * 1024 * 1024 / 512);
            bio->SetUblock(iter);
            bio->SetCallback(callback);
            IODispatcherSingleton::Instance()->Submit(bio);
            prevCtrlr = ctrlr;
            break;
        }
    }
    // during this time, abort command will be set.

    sleep(10);
    int testCountLocal = testCount;
    printf("callback : %d\n", testCountLocal);
    bool success = (testCountLocal == 1);
    libraryUnitTest.TestResult(1, success);
}

const uint32_t CALLBACK_COUNT = 1024;

void
DiskIo(UblockSharedPtr dev, void* ctx)
{
    struct spdk_nvme_ctrlr *ctrlr = nullptr, *prevCtrlr = nullptr;
    UblockSharedPtr targetDevice = nullptr;
    UnvmeSsd* ssd = nullptr;
    struct spdk_nvme_ns* ns;
    string sn;
    if (dev->GetType() == DeviceType::SSD)
    {
        if (firstdev == true)
            return;
        firstdev = true;
        printf("%s will be timed-out \n", dev->GetName());
        ssd = dynamic_cast<UnvmeSsd*>(dev.get());
        ns = ssd->GetNs();
        sn = dev->GetSN();

        ctrlr = spdk_nvme_ns_get_ctrlr(ns);
        spdk_nvme_ctrlr_register_timeout_callback(ctrlr, 0, 0,
            &Nvme::ControllerTimeoutCallback, nullptr);

        for (unsigned int i = 0; i < CALLBACK_COUNT; i++)
        {
            uint64_t size = 8 * 1024 / 512;

            void* mem = pos::Memory<512>::Alloc(size);

            UbioSmartPtr bio(new Ubio(mem, size, 0));
            CallbackSmartPtr callback(new DummyCallbackHandler(false));
            bio->dir = UbioDir::Write;

            bio->SetLba(512 * 1024 * 1024 / 512);
            bio->SetUblock(dev);

            bio->SetCallback(callback);
            IODispatcherSingleton::Instance()->Submit(bio);
        }
        // We wait that callback is called for all IOs
        sleep(20);
    }
}

void
test2_io_multiple_abort()
{
    clean_test();
    libraryUnitTest.TestStart(2);
    IODispatcher& ioDispatcher = *IODispatcherSingleton::Instance();

    DeviceManagerSingleton::Instance()->IterateDevicesAndDoFunc(DiskIo, nullptr);

    // We wait for detaching the device from the device manager.
    sleep(50);

    int testCountLocal = testCount;

    printf("callback : %d\n", testCountLocal);
    libraryUnitTest.TestResult(2, testCount == CALLBACK_COUNT);
}

} // namespace pos

int
main(int argc, char *argv[])
{
    libraryUnitTest.Initialize(argc, argv, "../../../../");
    pos::test1_iotimeout_abort();
    pos::test2_io_multiple_abort();
    libraryUnitTest.SuccessAndExit();
    return 0;
}
