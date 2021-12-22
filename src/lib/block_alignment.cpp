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

#include "src/lib/block_alignment.h"

#include "src/include/memory.h"

namespace pos
{
BlockAlignment::BlockAlignment(uint64_t originalStartAddress, uint64_t size)
: startAddress(originalStartAddress),
  headSize(0)
{
    uint64_t endAddress = startAddress + size;

    headPosition = GetByteOffsetInBlock(startAddress);

    if (headPosition > 0)
    {
        headSize = BLOCK_SIZE - headPosition;

        if (headSize > size)
        {
            headSize = size;
        }
    }

    if (headSize != 0)
    {
        startAddress -= headPosition;
    }

    tailSize = GetByteOffsetInBlock(endAddress);
    if (tailSize != 0)
    {
        endAddress += BLOCK_SIZE - tailSize;
    }

    blockCount = ChangeByteToBlock(endAddress - startAddress);
}

bool
BlockAlignment::_IsFirstBlock(uint32_t blockIndex)
{
    return blockIndex == 0;
}

bool
BlockAlignment::_IsLastBlock(uint32_t blockIndex)
{
    return blockIndex == (blockCount - 1);
}

uint64_t
BlockAlignment::GetHeadBlock(void)
{
    return ChangeByteToBlock(startAddress);
}

uint64_t
BlockAlignment::GetTailBlock(void)
{
    return GetHeadBlock() + blockCount - 1;
}

uint32_t
BlockAlignment::GetHeadPosition(void)
{
    return headPosition;
}

uint32_t
BlockAlignment::GetDataSize(uint32_t blockIndex)
{
    uint32_t dataSize = BLOCK_SIZE;
    if (_IsFirstBlock(blockIndex))
    {
        if (HasHead())
        {
            dataSize = GetHeadSize();
            return dataSize;
        }
    }

    if (_IsLastBlock(blockIndex))
    {
        if (HasTail())
        {
            dataSize = GetTailSize();
        }
    }

    return dataSize;
}

uint32_t
BlockAlignment::GetBlockCount(void)
{
    return blockCount;
}

uint32_t
BlockAlignment::GetHeadSize(void)
{
    return headSize;
}

uint32_t
BlockAlignment::GetTailSize(void)
{
    return tailSize;
}

bool
BlockAlignment::HasTail(void)
{
    return tailSize != 0;
}

bool
BlockAlignment::HasHead(void)
{
    return headSize != 0;
}

uint64_t
BlockAlignment::AlignHeadLba(uint32_t blockIndex, uint64_t originalLba)
{
    if (_IsFirstBlock(blockIndex) && HasHead())
    {
        return originalLba + ChangeByteToSector(headPosition);
    }

    return originalLba;
}

} // namespace pos
