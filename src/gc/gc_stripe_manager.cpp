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

#include "src/gc/gc_stripe_manager.h"

#include "src/allocator_service/allocator_service.h"
#include "src/io/general_io/translator.h"
#include "src/include/meta_const.h"
#include "src/gc/gc_flush_submission.h"

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/i_block_allocator.h"

#include <utility>

namespace pos
{
GcStripeManager::GcStripeManager(IArrayInfo* array)
: array(array)
{
    arrayName = array->GetName();
    iBlockAllocator = AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayName);
    iWBStripeAllocator = AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName);
}

GcStripeManager::~GcStripeManager(void)
{
}

std::pair <VirtualBlks, Stripe*>
GcStripeManager::AllocateBlocks(uint32_t volumeId, uint32_t remainCount)
{
    VirtualBlks vsas = iBlockAllocator->AllocateWriteBufferBlks(volumeId,
                remainCount, true);
    Stripe* stripe = nullptr;
    if (IsUnMapVsa(vsas.startVsa) == false)
    {
        Translator translator(vsas.startVsa, arrayName);
        StripeAddr lsidEntry = translator.GetLsidEntry(0);
        stripe = iWBStripeAllocator->GetStripe(lsidEntry);
    }

    return make_pair(vsas, stripe);
}

} // namespace pos
