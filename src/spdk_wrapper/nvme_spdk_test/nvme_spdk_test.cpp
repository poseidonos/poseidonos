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

int initialize_ibofos(int argc, char** argv);
extern int argc;
extern char* argv;
struct spdk_nvme_ns;

namespace pos
{
volatile int testCount = 0;
IOErrorType retError;
volatile int errCount = 0;

// Test 1 Indicates

void
clean_test(void)
{
    testCount = 0;
    errCount = 0;
}

void
test_start(int i)
{
    printf("### test %d starts ####\n", i);
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
test1_subsystem_reset(void)
{
    test_start(1);
    std::vector<UblockSharedPtr> devs = DeviceManagerSingleton::Instance()->GetDevs();
    struct spdk_nvme_ctrlr* ctrlr = nullptr;
    UblockSharedPtr targetDevice = nullptr;
    for (auto& iter : devs)
    {
        if (iter->GetType() == DeviceType::SSD)
        {
            printf("%s will be Nssr \n", iter->GetName());
            UnvmeSsd* ssd = dynamic_cast<UnvmeSsd*>(iter);
            struct spdk_nvme_ns* ns = ssd->GetNs();
            ctrlr = spdk_nvme_ns_get_ctrlr(ns);
            targetDevice = iter;
            break;
        }
    }
    volatile struct spdk_nvme_registers* reg =
        spdk_nvme_ctrlr_get_registers(ctrlr);
    printf("NSSR Support %d \n", reg->cap.bits.nssrs);
    if (reg->cap.bits.nssrs == 0)
    {
        test_success(1, true);
        return;
    }
    Nvme::SubsystemReset(ctrlr);
    printf("NSSR triggered, Wait CAP.TO %d second \n", (reg->cap.bits.to + 1) / 2);
    sleep(reg->cap.bits.to);

    devs = DeviceManagerSingleton::Instance()->GetDevs();

    for (auto& iter : devs)
    {
        if (iter == targetDevice)
        {
            test_success(1, false);
            return;
        }
    }
    test_success(1, true);
}

void
test2_ctrl_reset(void)
{
    test_start(2);
    std::vector<UblockSharedPtr> devs = DeviceManagerSingleton::Instance()->GetDevs();
    struct spdk_nvme_ctrlr* ctrlr = nullptr;
    UblockSharedPtr targetDevice = nullptr;
    UnvmeSsd* ssd = nullptr;
    struct spdk_nvme_ns* ns;
    string sn;
    for (auto& iter : devs)
    {
        if (iter->GetType() == DeviceType::SSD)
        {
            printf("%s will be resetted \n", iter->GetName());
            ssd = dynamic_cast<UnvmeSsd*>(iter);
            ns = ssd->GetNs();
            sn = iter->GetSN();
            ctrlr = spdk_nvme_ns_get_ctrlr(ns);
            targetDevice = iter;
            break;
        }
    }
    volatile struct spdk_nvme_registers* reg =
        spdk_nvme_ctrlr_get_registers(ctrlr);

    Nvme* nvmeSsd = new Nvme("Test");
    nvmeSsd->Pause();

    UnvmeDrvSingleton::Instance()->DeviceDetached(targetDevice->GetSN());

    while (nvmeSsd->IsPaused())
    {
        usleep(1);
    }

    nvmeSsd->Pause();

    int ret = spdk_nvme_ctrlr_reset(ctrlr);
    printf("ret = %d \n", ret);

    Nvme::SpdkDetach(ns);
    nvmeSsd->Resume();

    // while(nvmeSsd->IsPaused());
    sleep(10);

    devs = DeviceManagerSingleton::Instance()->GetDevs();

    for (auto& iter : devs)
    {
        if (iter == targetDevice)
        {
            test_success(2, false);
            return;
        }
        if (iter->GetSN() == sn)
        {
            printf("Attach Device for %s is Successful\n", sn.c_str());
            test_success(2, true);
            return;
        }
    }
    test_success(2, false);
}

} // namespace pos

int
main(void)
{
    char* argvPtr = argv;
    std::future<int> f = std::async(std::launch::async, initialize_ibofos, argc, &argvPtr);
    printf("Please use ./setup_ibofos_nvmf_volume.sh command before pressing any key. If you ready please press Any key");
    std::cin.get();

    pos::test1_subsystem_reset();
    pos::test2_ctrl_reset();

    assert(0);
    return 0;
}
