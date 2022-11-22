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

#ifndef __VOLUME_PERFOMANCE_PROPERTY__
#define __VOLUME_PERFOMANCE_PROPERTY__

#include <cstdint>
#include <string>

using namespace std;
namespace pos
{

const uint32_t KIOPS = 1000;
const uint32_t MIB_IN_BYTE = 1024 * 1024;
const uint64_t MIN_IOPS_LIMIT = 10;
const uint64_t MIN_BW_LIMIT = 10;
const uint64_t MAX_IOPS_LIMIT = UINT64_MAX / KIOPS;
const uint64_t MAX_BW_LIMIT = UINT64_MAX / MIB_IN_BYTE;

class PerfomanceProperty
{
public:
    PerfomanceProperty(uint64_t maxiops, uint64_t maxbw, uint64_t miniops, uint64_t minbw);
    ~PerfomanceProperty(void);
    
    uint64_t GetMaxIOPS(void) {return maxiops;}
    uint64_t GetMaxBW(void) {return maxbw;}
    uint64_t GetMinIOPS(void) {return miniops;}
    uint64_t GetMinBW(void) {return minbw;}

    void SetMaxIOPS(uint64_t val);
    void SetMaxBW(uint64_t val);
    void SetMinIOPS(uint64_t val);
    void SetMinBW(uint64_t val);

private:
    // 0 == unlimited
    uint64_t maxiops;
    uint64_t maxbw;
    uint64_t miniops;
    uint64_t minbw;
};

}

#endif //__VOLUME_PERFOMANCE_PROPERTY__
