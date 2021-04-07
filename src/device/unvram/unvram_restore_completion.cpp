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

#include "unvram_restore_completion.h"
#include "src/include/ibof_event_id.h"
#include "src/include/ibof_event_id.hpp"
#include "src/array/array.h"
namespace ibofos
{

std::atomic<uint32_t> UnvramRestoreCompletion::pendingUbio;

UnvramRestoreCompletion::UnvramRestoreCompletion(UbioSmartPtr ubio)
: Callback(false),
    ubio(ubio)
{
}

UnvramRestoreCompletion::~UnvramRestoreCompletion(void)
{
}

void UnvramRestoreCompletion::IncreasePendingUbio(void)
{
    pendingUbio++;
}

void UnvramRestoreCompletion::WaitPendingUbioZero(void)
{
     while (pendingUbio != 0)
     {
         usleep(1);
     }
}


bool UnvramRestoreCompletion::_DoSpecificJob(void)
{
    const uint32_t bytesPerHugepage = 2 * SZ_1MB;
    uint32_t unitsPerHugepage = bytesPerHugepage / Ubio::BYTES_PER_UNIT;

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVRAM_RESTORING_PAGE_DONE;

    delete ubio->GetDev();

    pendingUbio--;
    IBOF_TRACE_INFO((int)eventId,
        IbofEventId::GetString(eventId),
        (ubio->GetPba().lba / unitsPerHugepage));

    return true;
}

} // namespace ibofos
