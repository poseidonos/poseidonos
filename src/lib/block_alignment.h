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

#include <cstdint>

namespace pos
{
class BlockAlignment
{
public:
    BlockAlignment(uint64_t startAddress, uint64_t size);
    virtual ~BlockAlignment(void)
    {
    }
    virtual uint32_t GetBlockCount(void);
    virtual uint32_t GetHeadSize(void);
    virtual uint32_t GetTailSize(void);
    virtual bool HasTail(void);
    virtual bool HasHead(void);
    virtual uint64_t AlignHeadLba(uint32_t blockIndex, uint64_t originalLba);
    virtual uint32_t GetDataSize(uint32_t blockIndex);
    virtual uint64_t GetHeadBlock(void);
    virtual uint64_t GetTailBlock(void);
    virtual uint32_t GetHeadPosition(void);

private:
    uint64_t startAddress;
    uint32_t headSize;
    uint32_t tailSize;
    uint32_t blockCount;
    uint32_t headPosition;

    bool _IsFirstBlock(uint32_t blockIndex);
    bool _IsLastBlock(uint32_t blockIndex);
};

} // namespace pos
