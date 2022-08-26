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

#include "src/journal_manager/config/log_buffer_layout.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
LogBufferLayout::LogBufferLayout(void)
{
    _Reset();
}

void
LogBufferLayout::_Reset(void)
{
    groupLayout.clear();
}

void
LogBufferLayout::Init(uint64_t logBufferSize, int numLogGroups)
{
    _Reset();

    uint64_t startOffset = 0;
    uint64_t logGroupSize = logBufferSize / numLogGroups;

    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        LogGroupLayout layout;
        layout.startOffset = startOffset;
        layout.maxOffset = startOffset + logGroupSize;
        layout.footerStartOffset = layout.maxOffset - sizeof(LogGroupFooter);

        groupLayout.push_back(layout);

        startOffset = layout.maxOffset;
    }
}

LogGroupLayout
LogBufferLayout::GetLayout(int logGroupId)
{
    if (static_cast<int>(groupLayout.size()) <= logGroupId)
    {
        POS_TRACE_DEBUG(EID(JOURNAL_DEBUG),
            "Cannot get log buffer layout of group {}, max group is {}", logGroupId, groupLayout.size() - 1);

        return LogGroupLayout();
    }
    else
    {
        return groupLayout[logGroupId];
    }
}
} // namespace pos
