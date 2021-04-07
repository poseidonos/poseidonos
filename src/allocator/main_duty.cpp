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

#include "main_duty.h"

#include <vector>

#include "common_duty.h"
#include "io_duty.h"
#include "src/array/array.h"
#include "src/io/backend_io/flush_read_submission.h"
#include "src/mapper/mapper.h"
#include "src/volume/volume_manager.h"
namespace ibofos
{
MainDuty::MainDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI,
    CommonDuty* commonDutyI, IoDuty* ioDutyI)
: addrInfo(addrInfoI),
  meta(metaI),
  commonDuty(commonDutyI),
#if defined NVMe_FLUSH_HANDLING
  ioDuty(ioDutyI),
  flushCmdManager(FlushCmdManagerSingleton::Instance())
#else
  ioDuty(ioDutyI)
#endif
{
}

MainDuty::~MainDuty()
{
}

void
MainDuty::FlushAllUserdata()
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    std::string arrayState = ArraySingleton::Instance()->GetCurrentStateStr();
    if (arrayState == "BROKEN")
    {
        return;
    }

    commonDuty->CheckAllActiveStripes(stripesToFlush, vsidToCheckFlushDone);
    FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);

    for (auto it = commonDuty->GetwbStripeArrayBegin(); it != commonDuty->GetwbStripeArrayEnd(); ++it)
    {
        while ((*it)->GetBlksRemaining() > 0)
        {
            usleep(1);
        }
        while ((*it)->IsFinished() == false)
        {
            usleep(1);
        }
    }
}

void
MainDuty::FlushAllUserdataWBT()
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    ioDuty->TurnOffBlkAllocation();
    commonDuty->CheckAllActiveStripes(stripesToFlush, vsidToCheckFlushDone);
    ioDuty->TurnOnBlkAllocation();

    FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);
}

int
MainDuty::RequestStripeFlush(Stripe& stripe)
{
    EventSmartPtr event(new FlushReadSubmission(&stripe));
    return stripe.Flush(event);
}

void
MainDuty::FinalizeWriteIO(std::vector<Stripe*>& stripesToFlush,
    std::vector<StripeId>& vsidToCheckFlushDone)
{
    // Wait for write I/O residues on active stripes then flush
    Mapper& mapper = *MapperSingleton::Instance();

    for (auto& stripe : stripesToFlush)
    {
        while (stripe->GetBlksRemaining() > 0)
        {
            usleep(1);
        }
    }

    // Check if flushing has been completed
    for (auto vsid : vsidToCheckFlushDone)
    {
        Stripe* stripe = nullptr;
        do
        {
            StripeAddr lsa = mapper.GetLSA(vsid);
            stripe = commonDuty->GetStripe(lsa);
        } while (stripe != nullptr);
    }
}

#if defined NVMe_FLUSH_HANDLING
void
MainDuty::GetAllActiveStripes(int volumeId)
{
    ioDuty->TurnOffBlkAllocation();
    if (VolumeManagerSingleton::Instance()->GetVolumeStatus(volumeId) == Mounted)
    {
        commonDuty->PickActiveStripe(volumeId, flushCmdManager->stripesToFlush, flushCmdManager->vsidToCheckFlushDone);
    }
}

// TODO (karthik.di) Cleanup of unnecessary codes
// No need to wait for partial stripes flush completion as it will be done in WaitForAllStripesFlush
bool
MainDuty::FlushPartialStripes()
{
    std::vector<Stripe*>& stripesToFlush = flushCmdManager->stripesToFlush;

    auto stripe = stripesToFlush.begin();
    while (stripe != stripesToFlush.end())
    {
        if ((*stripe)->GetBlksRemaining() == 0)
        {
            if ((*stripe)->IsFinished() == false)
            {
                // RequestStripeFlush(**stripe);
            }
            stripe = stripesToFlush.erase(stripe);
        }
        else
        {
            // This stripe will be flushed in Write Completion
            stripe = stripesToFlush.erase(stripe);
        }
    }

    return stripesToFlush.size() == 0;
}

bool
MainDuty::WaitForPartialStripesFlush()
{
    Mapper& mapper = *MapperSingleton::Instance();

    std::vector<StripeId>& vsidToCheckFlushDone = flushCmdManager->vsidToCheckFlushDone;

    // Check if flushing has been completed
    auto vsid = vsidToCheckFlushDone.begin();
    while (vsid != vsidToCheckFlushDone.end())
    {
        Stripe* stripe = nullptr;
        StripeAddr lsa = mapper.GetLSA(*vsid);
        stripe = commonDuty->GetStripe(lsa);

        if (stripe != nullptr)
        {
            vsid++;
        }
        else
        {
            vsid = vsidToCheckFlushDone.erase(vsid);
        }
    }

    return (vsidToCheckFlushDone.size() == 0);
}

bool
MainDuty::WaitForAllStripesFlush(uint32_t volumeId)
{
    Mapper& mapper = *MapperSingleton::Instance();
    // Need to check for stripes belonging to requested volumes stipes only including GC stripe of that volume
    uint32_t volumeIdGC = volumeId + MAX_VOLUME_COUNT;

    for (auto it = commonDuty->GetwbStripeArrayBegin(); it != commonDuty->GetwbStripeArrayEnd(); ++it)
    {
        if (volumeId != (*it)->GetAsTailArrayIdx() || volumeIdGC != (*it)->GetAsTailArrayIdx())
        {
            continue;
        }
        if ((*it)->GetBlksRemaining() > 0)
        {
            if ((*it)->GetBlksRemaining() + meta->GetActiveStripeTail(volumeId).offset == addrInfo.GetblksPerStripe())
            {
                continue;
            }
        }

        Stripe* stripe = nullptr;
        StripeAddr lsa = mapper.GetLSA((*it)->GetVsid());
        stripe = commonDuty->GetStripe(lsa);

        if (stripe != nullptr)
        {
            return false;
        }
    }

    return true;
}

void
MainDuty::UnblockAllocating()
{
    ioDuty->TurnOnBlkAllocation();
}
#endif

} // namespace ibofos
