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

using namespace std;
namespace pos
{

enum class RaidTypeEnum
{
    NOT_SUPPORTED,
    NONE,
    RAID0,
    RAID5,
    RAID10,
    RAID6,
    TYPE_COUNT,
};

class RaidType
{
public:
    RaidType(void) { val = RaidTypeEnum::NONE; }
    RaidType(RaidTypeEnum t) : val(t) { }
    RaidType(string type)
    {
        val = RaidTypeEnum::NOT_SUPPORTED;
        for (int i = 0; i < (int)RaidTypeEnum::TYPE_COUNT; i++)
        {
            if (type == RAID_STR[i])
            {
                val = static_cast<RaidTypeEnum>(i);
                break;
            }
        }
    }
    operator RaidTypeEnum(void) const { return val; }
    bool operator == (const RaidType t) const { return val == t.val; }
    bool operator != (const RaidType t) const { return val != t.val; }
    bool operator == (const RaidTypeEnum t) const { return val == t; }
    bool operator != (const RaidTypeEnum t) const { return val != t; }
    string ToString(void) const { return RAID_STR[(int)val]; }

private:
    RaidTypeEnum val;
    string RAID_STR[(int)RaidTypeEnum::TYPE_COUNT] =
    {
        "NOT_SUPPORTED",
        "NONE",
        "RAID0",
        "RAID5",
        "RAID10",
        "RAID6",
    };
};

} // namespace pos
