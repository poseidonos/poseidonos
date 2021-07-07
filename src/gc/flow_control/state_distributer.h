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

#pragma once

#include "src/gc/flow_control/token_distributer.h"
#include <tuple>

namespace pos
{
class StateDistributer : public TokenDistributer
{
public:
    StateDistributer(uint32_t totalToken, uint32_t gcThreshold,
                    uint32_t targetSegment, uint32_t targetPercent,
                    uint32_t urgentSegment, uint32_t urgentPercent,
                    uint32_t totalTokenInStripe, uint32_t blksPerStripe);
    ~StateDistributer(void) override;
    std::tuple<uint32_t, uint32_t> Distribute(uint32_t freeSegments) override;

private:
    uint32_t totalToken = 0;
    uint32_t gcThreshold = 0;
    uint32_t targetSegment = 0;
    uint32_t targetPercent = 0;
    uint32_t urgentSegment = 0;
    uint32_t urgentPercent = 0;
    uint32_t totalTokenInStripe = 0;
    uint32_t blksPerStripe = 0;
};

}; // namespace pos
