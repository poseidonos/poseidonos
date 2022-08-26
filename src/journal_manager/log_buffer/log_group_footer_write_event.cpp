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
*   
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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

#include "src/journal_manager/log_buffer/log_group_footer_write_event.h"

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log_buffer/i_journal_log_buffer.h"
#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"
#include "src/logger/logger.h"

namespace pos
{
LogGroupFooterWriteEvent::LogGroupFooterWriteEvent(IJournalLogBuffer* logBuffer,
    LogGroupFooter footer, uint64_t footerOffset,
    int logGroupId, EventSmartPtr callback)
: logBuffer(logBuffer),
  offset(footerOffset),
  footer(footer),
  logGroupId(logGroupId),
  callback(callback)
{
}

bool
LogGroupFooterWriteEvent::Execute(void)
{
    LogGroupFooterWriteContext* context = new LogGroupFooterWriteContext(logGroupId, callback);
    context->SetIoRequest(offset, footer);

    int result = logBuffer->InternalIo(context);
    if (result != 0)
    {
        POS_TRACE_DEBUG(EID(JOUNRAL_WRITE_LOG_GROUP_FOOTER),
            "Failed to write log group footer");
        return false;
    }
    else
    {
        POS_TRACE_DEBUG(EID(JOUNRAL_WRITE_LOG_GROUP_FOOTER),
            "Write log group footer (id {}, version {})", logGroupId, footer.lastCheckpointedSeginfoVersion);
        return true;
    }
}
} // namespace pos
