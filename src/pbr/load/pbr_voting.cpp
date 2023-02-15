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

#include "pbr_voting.h"
#include "src/helper/enumerable/query.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"
#include <algorithm>

namespace pbr
{
bool comparePbrCandidate(PbrCandidate *a, PbrCandidate *b)
{
    if (a->GetNumOfVotes() == b->GetNumOfVotes())
    {
        return a->GetId() > b->GetId();
    }
    return a->GetNumOfVotes() > b->GetNumOfVotes();
}

PbrVoting::~PbrVoting()
{
    for (auto d : districts)
    {
        for (auto candidate : d.second)
        {
            delete candidate;
        }
    }
}

void
PbrVoting::Vote(AteData* candidate)
{
    string arrayUuid = candidate->arrayUuid;
    uint64_t id = candidate->lastUpdatedDateTime;

    auto it = districts.find(arrayUuid);
    if (it == districts.end())
    {
        districts.emplace(arrayUuid, vector<PbrCandidate*>{ new PbrCandidate(candidate) });
    }
    else
    {
        vector<PbrCandidate*>& candidates = it->second;
        PbrCandidate* pc = Enumerable::First(candidates,
            [id](auto c) { return c->GetId() == id; });
        if (pc == nullptr)
        {
            candidates.push_back(new PbrCandidate(candidate));
        }
        else
        {
            pc->Vote();
        }
    }
}

map<string, AteData*>
PbrVoting::Poll(void)
{
    map<string, AteData*> winners;
    for (auto d : districts)
    {
        string arrayUuid = d.first;
        auto candidates = d.second;
        auto winner = candidates.front();
        if (candidates.size() > 1)
        {
            POS_TRACE_WARN(EID(PBR_NEED_VOTING), "array_uuid:{}, num_of_candidates:{}",
                arrayUuid, candidates.size());
            sort(candidates.begin(), candidates.end(), comparePbrCandidate);
            winner = candidates.front();
            POS_TRACE_WARN(EID(PBR_VOTING_ELECTED), "array_uuid:{}, last_updated_dt:{}",
                arrayUuid, winner->GetId());
        }
        winners.emplace(arrayUuid, winner->GetAteData());
    }
    return winners;
}
} // namespace pbr
