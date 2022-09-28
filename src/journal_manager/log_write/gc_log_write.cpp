/*
*   BSD LICENSE
*   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/journal_manager/log_write/gc_log_write.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/journal_manager/log_write/gc_log_write_completed.h"
#include "src/journal_manager/log_write/log_write_handler.h"
#include "src/logger/logger.h"

namespace pos
{
GcLogWrite::GcLogWrite()
: GcLogWrite({}, nullptr, nullptr)
{
}

GcLogWrite::GcLogWrite(std::vector<LogWriteContext*> blockContexts_,
    EventSmartPtr gcLogWriteCompleted_,
    LogWriteHandler* logWrite)
: Callback(false, CallbackType_GcLogWrite),
  blockContexts(blockContexts_),
  gcLogWriteCompleted(gcLogWriteCompleted_),
  logWriteHandler(logWrite),
  totalNumBlockLogs(blockContexts_.size())
{
}

bool
GcLogWrite::_DoSpecificJob(void)
{
    if (totalNumBlockLogs == 0)
    {
        return gcLogWriteCompleted->Execute();
    }
    else
    {
        for (auto it = blockContexts.begin(); it != blockContexts.end();)
        {
            int result = logWriteHandler->AddLog(*it);
            if (result == 0)
            {
                it = blockContexts.erase(it);
            }
            else if (result < 0)
            {
                POS_TRACE_ERROR(EID(JOURNAL_WRITE_FAILED), "GC log write failed, will be re-tried, reason: {}", result);
                break;
            }
            else
            {
                it++;
            }
        }

        // GC journal write should be re-tried when there's pending journal
        return (blockContexts.size() == 0);
    }
}

} // namespace pos
