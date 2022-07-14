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

enum class RebuildTypeEnum
{
    BASIC,
    QUICK,
    TYPE_COUNT,
};

class RebuildType
{
public:
    RebuildType(void) { val = RebuildTypeEnum::BASIC; }
    RebuildType(RebuildTypeEnum t) : val(t) {}
    RebuildType(string type)
    {
        val = RebuildTypeEnum::BASIC;
        for (int i = 0; i < (int)RebuildTypeEnum::TYPE_COUNT; i++)
        {
            if (type == REBUILDTYPE_STR[i])
            {
                val = static_cast<RebuildTypeEnum>(i);
                break;
            }
        }
    }
    operator RebuildTypeEnum(void) const { return val; }
    bool operator == (const RebuildType t) const { return val == t.val; }
    bool operator != (const RebuildType t) const { return val != t.val; }
    bool operator == (const RebuildTypeEnum t) const { return val == t; }
    bool operator != (const RebuildTypeEnum t) const { return val != t; }
    string ToString(void) const { return REBUILDTYPE_STR[(int)val]; }

private:
    RebuildTypeEnum val;
    string REBUILDTYPE_STR[(int)RebuildTypeEnum::TYPE_COUNT] =
    {
        "BASIC",
        "QUICK",
    };
};

} // namespace pos
