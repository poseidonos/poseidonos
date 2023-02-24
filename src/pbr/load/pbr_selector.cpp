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

#include "pbr_selector.h"
#include "src/logger/logger.h"
#include <algorithm>

using namespace std;

namespace pbr
{
int
PbrSelector::Select(vector<unique_ptr<AteData>>& candidiates,
    unique_ptr<PbrVoting> voting)
{
    int ret = 0;
    if (candidiates.size() == 0)
    {
        ret = EID(PBR_VOTING_NO_CANDIDATE);
        POS_TRACE_WARN(ret, "");
        return ret;
    }
    for (auto& candidate : candidiates)
    {
        voting->Vote(candidate.get());
    }
    auto winners = voting->Poll();
    if (winners.size() == 0)
    {
        ret = EID(PBR_VOTING_NO_WINNER);
        POS_TRACE_WARN(ret, "num_of_candidates:{}", candidiates.size());
    }
    for (auto it = candidiates.begin(); it != candidiates.end();)
    {
        bool isWinner = false;
        for (auto winner : winners)
        {
            if (winner.second == (*it).get())
            {
                isWinner = true;
                break;
            }
        }
        if (isWinner == false)
        {
            it = candidiates.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return ret;
}
} // namespace pbr
