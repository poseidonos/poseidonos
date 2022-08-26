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

#include "rebuild_read_intermediate_complete_handler.h"

#include "src/bio/ubio.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
RebuildReadIntermediateCompleteHandler::
    RebuildReadIntermediateCompleteHandler(UbioSmartPtr input)
: Callback(false, CallbackType_RebuildReadIntermediateCompleteHandler),
  ubio(input)
{
}

RebuildReadIntermediateCompleteHandler::
    ~RebuildReadIntermediateCompleteHandler(void)
{
}

bool
RebuildReadIntermediateCompleteHandler::_DoSpecificJob(void)
{
    if (unlikely(nullptr == ubio))
    {
        POS_EVENT_ID eventId = EID(RDCMP_INVALID_UBIO);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Ubio is null at ReadCompleteHandler");
        return true;
    }

    if (unlikely(_GetErrorCount()))
    {
        POS_EVENT_ID eventId = EID(RDCMP_READ_FAIL);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Uncorrectable data error");
    }

    ubio = nullptr;

    return true;
}

} // namespace pos
