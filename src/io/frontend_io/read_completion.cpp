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

#include "src/io/frontend_io/read_completion.h"

#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/block_alignment.h"
#include "src/io/general_io/volume_io.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
ReadCompletion::ReadCompletion(VolumeIoSmartPtr input)
: Callback(true),
  volumeIo(input)
{
}

ReadCompletion::~ReadCompletion()
{
}

bool
ReadCompletion::_DoSpecificJob(void)
{
    try
    {
        if (unlikely(nullptr == volumeIo))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::RDCMP_INVALID_UBIO;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
            throw eventId;
        }

        if (unlikely(_GetErrorCount()))
        {
            // After rebuild, if error is still left,
            // we need to just leave the error.
            // this error will be automatically transfered to AIO
            // with callback mechanism.
            // Check Partition type and ft method
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::RDCMP_READ_FAIL;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
        }

        StripeAddr lsidEntry = volumeIo->GetLsidEntry();
        if (false == IsUnMapStripe(lsidEntry.stripeId))
        {
            uint64_t byteRba = ChangeSectorToByte(volumeIo->GetRba());
            BlockAlignment blockAlignment(byteRba, volumeIo->GetSize());
            uint32_t blockCount = blockAlignment.GetBlockCount();
            MapperSingleton::Instance()->DereferLsid(lsidEntry, blockCount);
        }
    }
    catch (...)
    {
    }
    volumeIo = nullptr;
    return true;
}

} // namespace ibofos
