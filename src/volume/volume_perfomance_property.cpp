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

#include "volume_perfomance_property.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace std;
namespace pos
{

PerfomanceProperty::PerfomanceProperty(uint64_t maxiops, uint64_t maxbw, uint64_t miniops, uint64_t minbw)
: maxiops(maxiops),
  maxbw(maxbw),
  miniops(miniops),
  minbw(minbw)
{
}

PerfomanceProperty::~PerfomanceProperty(void)
{

}

void
PerfomanceProperty::SetMaxIOPS(uint64_t val)
{
    if ((val != 0 && val < MIN_IOPS_LIMIT) || val > MAX_IOPS_LIMIT)
    {
        throw EID(VOL_REQ_QOS_OUT_OF_RANGE);
    }
    maxiops = val;
}

void
PerfomanceProperty::SetMaxBW(uint64_t val)
{
    if ((val != 0 && val < MIN_BW_LIMIT) || val > MAX_BW_LIMIT)
    {
        throw EID(VOL_REQ_QOS_OUT_OF_RANGE);
    }
    maxbw = val;
}

void
PerfomanceProperty::SetMinIOPS(uint64_t val)
{
    if (val != 0 && val > MAX_IOPS_LIMIT)
    {
        throw EID(VOL_REQ_QOS_OUT_OF_RANGE);
    }
    miniops = val;
}

void
PerfomanceProperty::SetMinBW(uint64_t val)
{
    if (val != 0 && val > MAX_BW_LIMIT)
    {
        throw EID(VOL_REQ_QOS_OUT_OF_RANGE);
    }
    minbw = val;
}

}
