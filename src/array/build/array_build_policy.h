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

#pragma once

#include <string>

#include "src/helper/string/string_checker.h"
#include "src/include/raid_type.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace std;

namespace pos
{
inline int CheckArrayName(string name)
{
    const size_t MIN_LEN = 2;
    const size_t MAX_LEN = 63;
    const char SPACE = ' ';
    const char* ALLOWED_CHAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_- ";

    int ret = EID(SUCCESS);
    StringChecker checker(name);
    size_t len = checker.Length();
    if (len < MIN_LEN)
    {
        ret = EID(ARRAY_NAME_TOO_SHORT);
        POS_TRACE_WARN(ret, "name len: {}", len);
        return ret;
    }
    else if (len > MAX_LEN)
    {
        ret = EID(ARRAY_NAME_TOO_LONG);
        POS_TRACE_WARN(ret, "name len: {}", len);
        return ret;
    }

    if (checker.StartWith(SPACE) || checker.EndWith(SPACE))
    {
        ret = EID(ARRAY_NAME_START_OR_END_WITH_SPACE);
        POS_TRACE_WARN(ret, "name: {}", name);
        return ret;
    }

    if (checker.OnlyContains(ALLOWED_CHAR) == false)
    {
        ret = EID(ARRAY_NAME_INCLUDES_SPECIAL_CHAR);
        POS_TRACE_WARN(ret, "name allowed only: {}", ALLOWED_CHAR);
        return ret;
    }

    return ret;
}

inline int CheckRaidType(string metaRaid, string dataRaid, uint32_t spareCount)
{
    POS_TRACE_DEBUG(EID(ARRAY_BUILDER_RAID_POLICY_CHECK_REQUEST),
        "meta_raid_type:{}, data_raid_type:{}", metaRaid, dataRaid);
    int ret = 0;
    RaidType dataRaidType = RaidType(dataRaid);
    RaidType metaRaidType = RaidType(metaRaid);
    if (dataRaidType == RaidTypeEnum::NOT_SUPPORTED ||
        metaRaidType == RaidTypeEnum::NOT_SUPPORTED)
    {
        ret = EID(NOT_SUPPORTED_RAIDTYPE);
        POS_TRACE_WARN(ret, "meta_raid_type:{}, data_raid_type:{}", metaRaid, dataRaid);
        return ret;
    }
    bool canAddSpare = dataRaidType != RaidTypeEnum::NONE &&
        dataRaidType != RaidTypeEnum::RAID0;
    if (canAddSpare == false && spareCount > 0)
    {
        ret = EID(RAID_DOES_NOT_SUPPORT_SPARE_DEV);
        POS_TRACE_WARN(ret, "meta_raid_type:{}, data_raid_type:{}", metaRaid, dataRaid);
        return ret;
    }
    return ret;
}

} // namespace pos
