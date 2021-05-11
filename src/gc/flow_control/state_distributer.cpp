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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/gc/flow_control/state_distributer.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "assert.h"

namespace pos
{
StateDistributer::StateDistributer(uint32_t totalToken, uint32_t gcThreshold,
                            uint32_t targetSegment, uint32_t targetPercent,
                            uint32_t urgentSegment, uint32_t urgentPercent,
                            uint32_t totalTokenInStripe, uint32_t blksPerStripe)
: totalToken(totalToken),
  gcThreshold(gcThreshold),
  targetSegment(targetSegment),
  targetPercent(targetPercent),
  urgentSegment(urgentSegment),
  urgentPercent(urgentPercent),
  totalTokenInStripe(totalTokenInStripe),
  blksPerStripe(blksPerStripe)
{
}

StateDistributer::~StateDistributer(void)
{
}

std::tuple<uint32_t, uint32_t>
StateDistributer::Distribute(uint32_t freeSegments)
{
    uint32_t userStripe;
    uint32_t userToken;
    uint32_t gcToken;

    if (freeSegments > gcThreshold)
    {
        userToken = totalToken;
        gcToken = 0;
    }
    else if (freeSegments > targetSegment)
    {
        userStripe = (totalTokenInStripe * targetPercent) / 100;
        userToken =  userStripe * blksPerStripe;
        gcToken = totalToken - userToken;
    }
    else if (freeSegments > urgentSegment)
    {
        userStripe = (totalTokenInStripe * urgentPercent) / 100;
        userToken =  userStripe * blksPerStripe;
        gcToken = totalToken - userToken;
    }
    else
    {
        userToken = 0;
        gcToken = totalToken;
    }

    if ((userToken > totalToken) || (gcToken > totalToken))
    {
        POS_TRACE_DEBUG(EID(FC_TOKEN_OVERFLOW), "FlowControl distributed token more than totaltoken totalToken: {}, userToken: {}, gcToken: {}", totalToken, userToken, gcToken);
        assert(false);
    }

    return std::make_tuple(userToken, gcToken);
}

}; // namespace pos
