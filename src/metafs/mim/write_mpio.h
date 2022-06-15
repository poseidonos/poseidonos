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

#include "mpio.h"

namespace pos
{
class WriteMpio : public Mpio
{
public:
    explicit WriteMpio(void* mdPageBuf, const bool directAccessEnabled);
    virtual ~WriteMpio(void);
    virtual MpioType GetType(void) const override
    {
        return MpioType::Write;
    }

protected:
    virtual void _InitStateHandler(void) override;
    bool _MergeMDPage(void* userBuf, FileSizeType userWByteOffset, FileSizeType userWByteSize, void* mdpageBuf);
    bool _HandleError(MpAioState expNextState);

private:
    bool _Init(MpAioState expNextState);
    bool _MakeReady(MpAioState expNextState);
    bool _Write(const MpAioState expNextState);
    bool _MergeData(MpAioState expNextState);
    bool _PrepareWrite(MpAioState expNextState);
    bool _CompleteIO(MpAioState expNextState);

    MetaLpnType prevLpn;
    MetaLpnType currLpn;
    void* prevBuf;
    void* currBuf;
};
} // namespace pos
