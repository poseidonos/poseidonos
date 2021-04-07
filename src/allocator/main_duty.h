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

#pragma once

#include "allocator_meta_archive.h"
#include "stripe.h"

#if defined NVMe_FLUSH_HANDLING
#include "src/io/frontend_io/flush_command_manager.h"
#endif

#include <vector>
namespace ibofos
{
class ArrayDuty;
class CommonDuty;
class IoDuty;

class MainDuty
{
public:
    MainDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI,
        CommonDuty* commonDutyI, IoDuty* ioDuty);
    virtual ~MainDuty(void);

    void FlushAllUserdata(void);    // MainHandler::Unmount()
    void FlushAllUserdataWBT(void); // WBT Flush Command
    int RequestStripeFlush(Stripe& stripe);
    void FinalizeWriteIO(std::vector<Stripe*>& stripesToFlush,
        std::vector<StripeId>& vsidToCheckFlushDone);
#if defined NVMe_FLUSH_HANDLING
    void GetAllActiveStripes(int volumeId);
    bool FlushPartialStripes(void);
    bool WaitForPartialStripesFlush(void);
    bool WaitForAllStripesFlush(uint32_t volumeId);
    void UnblockAllocating(void);
#endif

private:
    AllocatorAddressInfo& addrInfo;
    AllocatorMetaArchive* meta;
    CommonDuty* commonDuty;
    IoDuty* ioDuty;
#if defined NVMe_FLUSH_HANDLING
    FlushCmdManager* flushCmdManager;
#endif
};

} // namespace ibofos
