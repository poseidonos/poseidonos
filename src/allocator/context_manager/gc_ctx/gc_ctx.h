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

#include "src/allocator/include/allocator_const.h"

namespace pos
{
class BlockAllocationStatus;

class GcCtx
{
public:
    GcCtx(void) = default;
    explicit GcCtx(BlockAllocationStatus* allocStatus, uint32_t arrayId);
    virtual ~GcCtx(void) = default;
    uint32_t GetNormalGcThreshold(void);
    uint32_t GetUrgentThreshold(void);
    void SetNormalGcThreshold(uint32_t inputThreshold);
    void SetUrgentThreshold(uint32_t inputThreshold);
    virtual GcMode GetCurrentGcMode(void);
    virtual GcMode UpdateCurrentGcMode(uint32_t numFreeSegments);

private:
    void _UpdateGcMode(pos::GcMode newGcMode);
    void _PrintInfo(pos::GcMode newGcMode, uint32_t numFreeSegments);

    uint32_t normalGcThreshold = 20;
    uint32_t urgentGcThreshold = 5;
    GcMode curGcMode;
    GcMode prevGcMode;
    BlockAllocationStatus* blockAllocStatus;
    uint32_t arrayId = 0;
};

} // namespace pos
