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

#include "mfs_top_init_test.h"

#include "metafs_log.h"
#include "os_header.h"

#define DPDK_RTE_LIB_USE 0

namespace pos
{
void
SetupTestEnv(int argc, char** argv)
{
    std::srand(0);

    // rte_eal_init is necessary to use DPDK RTE Ring library
    // In metafs, when you use RTE lockless Q(find mfs_lockless_q.h, metafs_q.h), you need to have code below.
    // However, you will get trouble during below execution when you try to run the unit test with valgrind.
    //  (please note that you have to patch valgrind tool in order to work with DPDK properly. https://github.com/bluca/valgrind-dpdk)
    // Because valgrind tool captures all the executions behind RTE EAL library and it will take so long.
    // That means, you are likely to find a way to make valgrind execution much faster
    // for instance, by skipping memcheck upon rte_eal_init, when you want to use RTE library.
    // Since I haven't found a proper way to resolve the issue and RTE Ring lib. is not used curently though, I temporary disable below
#if (1 == DPDK_RTE_LIB_USE)
    int ret = rte_eal_init(argc, argv);
    assert(ret >= 0);
#endif
}

void
MountMetaStorage(void)
{
    // test purpose...this code will be moved to meta filesystem service core soon...
    uint64_t metaStorageSizeInByte;
    metaStorageSizeInByte = (uint64_t)2 * (1024 * 1024 * 1024); // 5GB
    metaStorage->CreateMetaStore("POSArray", MetaStorageType::SSD, metaStorageSizeInByte, true);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "SSD Meta storage subsystem mounted..size={}GB",
        metaStorageSizeInByte / (1024 * 1024 * 1024));
    metaStorageSizeInByte = (uint64_t)512 * (1024 * 1024); // 1GB
    metaStorage->CreateMetaStore("POSArray", MetaStorageType::NVRAM, metaStorageSizeInByte, true);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM Meta storage subsystem mounted..size={}GB",
        metaStorageSizeInByte / (1024 * 1024 * 1024));
}
} // namespace pos
