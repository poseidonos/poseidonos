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

#include "volume_name_policy.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
void
VolumeNamePolicy::CheckVolumeName(string name)
{
    StringChecker checker(name);
    size_t len = checker.Length();

    if (len < MIN_LEN)
    {
        POS_TRACE_WARN(POS_EVENT_ID::VOL_NAME_TOO_SHORT,
            "Volume name must be at least {} characters", MIN_LEN);

        throw static_cast<int>(POS_EVENT_ID::VOL_NAME_TOO_SHORT);
    }
    else if (len > MAX_LEN)
    {
        POS_TRACE_WARN(POS_EVENT_ID::VOL_NAME_TOO_LONG,
            "Volume name must be less or equal than {} characters", MAX_LEN);
        throw static_cast<int>(POS_EVENT_ID::VOL_NAME_TOO_LONG);
    }

    if (checker.StartWith(SPACE) || checker.EndWith(SPACE))
    {
        POS_TRACE_WARN(POS_EVENT_ID::VOL_NAME_NOT_ALLOWED,
            "Blank cannot be placed at the beginning or end of a volume name");
        throw static_cast<int>(POS_EVENT_ID::VOL_NAME_NOT_ALLOWED);
    }

    if (checker.OnlyContains(ALLOWED_CHAR) == false)
    {
        POS_TRACE_WARN(POS_EVENT_ID::VOL_NAME_NOT_ALLOWED,
            "Special characters cannot be used as volume names");
        throw static_cast<int>(POS_EVENT_ID::VOL_NAME_NOT_ALLOWED);
    }
}

} // namespace pos
