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

#include "src/array_models/interface/i_array_info.h"
#include <stdint.h>
#include <string>

namespace pos
{
class AllocatorAddressInfo
{
public:
    AllocatorAddressInfo(void) = default;
    virtual ~AllocatorAddressInfo(void) = default;
    virtual void Init(IArrayInfo* iArrayInfo = nullptr);

    virtual uint32_t GetblksPerStripe(void) { return blksPerStripe; }
    virtual uint32_t GetchunksPerStripe(void) { return chunksPerStripe; }
    virtual uint32_t GetnumWbStripes(void) { return numWbStripes; }
    virtual uint32_t GetnumUserAreaStripes(void) { return numUserAreaStripes; }
    virtual uint32_t GetblksPerSegment(void) { return blksPerSegment; }
    virtual uint32_t GetstripesPerSegment(void) { return stripesPerSegment; }
    virtual uint32_t GetnumUserAreaSegments(void) { return numUserAreaSegments; }

    void SetblksPerStripe(uint32_t value) { blksPerStripe = value; }
    void SetchunksPerStripe(uint32_t value) { chunksPerStripe = value; }
    void SetnumWbStripes(uint32_t value) { numWbStripes = value; }
    void SetnumUserAreaStripes(uint32_t value) { numUserAreaStripes = value; }
    void SetblksPerSegment(uint32_t value) { blksPerSegment = value; }
    void SetstripesPerSegment(uint32_t value) { stripesPerSegment = value; }
    void SetnumUserAreaSegments(uint32_t value) { numUserAreaSegments = value; }
    void SetUT(bool ut) { isUT = ut; }
    virtual bool IsUT(void) { return isUT; }

private:
    uint32_t blksPerStripe;
    uint32_t chunksPerStripe;
    uint32_t numWbStripes;
    uint32_t numUserAreaStripes;
    uint32_t blksPerSegment;
    uint32_t stripesPerSegment;
    uint32_t numUserAreaSegments;
    bool isUT;
};

} // namespace pos
