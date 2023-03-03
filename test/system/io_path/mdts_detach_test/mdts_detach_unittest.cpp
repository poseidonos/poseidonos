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

#include <cstdio>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "spdk/nvme_spec.h"
#include "src/device/device_manager.h"
#include "src/spdk_wrapper/nvme.hpp"
#include "src/device/unvme/unvme_drv.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/main/poseidonos.h"
#include "src/io_scheduler/io_dispatcher.h"

pos::LibraryUnitTest libraryUnitTest;

struct spdk_nvme_ns;

namespace pos
{
void
DiskIo(UblockSharedPtr dev, void* ctx)
{
    uint64_t size = 32 * 1024 * 1024 / 512;
    static int test_count = 0;
    static bool detachFlag = false;
    test_count++;
    if (test_count > 100)
    {
        int testCase = *(int*)ctx;
        if (testCase == 1)
        {
            size = 32 * 1024 * 1024 / 512;
        }
        else
        {
            size = 4 * 1024 / 512;
        }
    }

    void* mem = pos::Memory<512>::Alloc(size);

    UbioSmartPtr bio(new Ubio(mem, size, 0));

    bio->dir = UbioDir::Write;
    bio->SetLba(512 * 1024 * 1024 / 512);
    bio->SetUblock(dev);

    IODispatcher& ioDispatcher = *IODispatcherSingleton::Instance();

    std::vector<UblockSharedPtr> devs = DeviceManagerSingleton::Instance()->GetDevs();
    struct spdk_nvme_ctrlr* ctrlr = nullptr;
    UblockSharedPtr targetDevice = nullptr;
    UnvmeSsdSharedPtr ssd = nullptr;
    struct spdk_nvme_ns* ns;
    string sn;

    if (test_count > 100 && detachFlag == false)
    {
        for (auto& iter : devs)
        {
            if (iter->GetType() == DeviceType::SSD)
            {
                printf("%s will be resetted \n", iter->GetName());
                ssd = dynamic_pointer_cast<UnvmeSsd>(iter);
                ns = ssd->GetNs();
                sn = iter->GetSN();
                ctrlr = spdk_nvme_ns_get_ctrlr(ns);
                spdk_nvme_ctrlr_fail_and_remove(ctrlr);
                detachFlag = true;
                break;
            }
        }
    }

    int ret = ioDispatcher.Submit(bio, true);

    pos::Memory<512>::Free(mem);
}

// 32MB -> detach -> 4K IO
void
test1_32mb_with_detaching(void)
{
    libraryUnitTest.TestStart(1);
    DeviceManager* devMgr = DeviceManagerSingleton::Instance();
    int testCase = 0;
    for (int i = 0; i < 100; i++)
    {
        int result = devMgr->IterateDevicesAndDoFunc(DiskIo, (void*)&testCase);
    }
    libraryUnitTest.TestResult(1, true);
}

// 32MB -> detach -> 32MB IO
void
test2_32mb_with_detaching(void)
{
    libraryUnitTest.TestStart(2);
    DeviceManager* devMgr = DeviceManagerSingleton::Instance();
    int testCase = 1;
    for (int i = 0; i < 100; i++)
    {
        int result = devMgr->IterateDevicesAndDoFunc(DiskIo, (void*)&testCase);
    }
    libraryUnitTest.TestResult(2, true);
}

} // namespace pos

int
main(int argc, char* argv[])
{
    libraryUnitTest.Initialize(argc, argv, "../../../../");
    pos::test1_32mb_with_detaching();
    pos::test2_32mb_with_detaching();
    libraryUnitTest.SuccessAndExit();
    return 0;
}
