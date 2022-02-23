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

#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/lib/bitmap.h"

#include <cmath>
#include <iostream>
#include <string.h>

namespace pos
{
/*
 *  ffsl - Find First bit Set in a Long
 *   0 = ffsl(0)
 *   1 = ffsl(0x1)
 *  32 = ffsl(0x8000,0000)
 *  64 = ffsl(0x8000,0000,0000,0000)
 */
#define ffzl(x) (ffsl((~x)))

uint64_t
BitMap::_GetMask(uint64_t numSetBits, uint64_t offset)
{
    return ((uint64_t)(std::pow(2, numSetBits)) - 1) << offset;
}

void
BitMap::PrintMap(void)
{
    for (uint32_t row = 0; row < numEntry; row++)
    {
        for (int col = BITMAP_ENTRY_BITS - 1; col >= 0; --col)
        {
            if (map[row] & (1ULL << col))
            {
                std::cout << "1";
            }
            else
            {
                std::cout << "0";
            }
        }
        std::cout << std::endl;
    }
}

bool
BitMap::IsSetBit(uint64_t bitOffset)
{
    uint64_t row = bitOffset / BITMAP_ENTRY_BITS;
    uint64_t col = bitOffset % BITMAP_ENTRY_BITS;
    return (map[row] & (1ULL << col)) != 0;
}

uint64_t
BitMap::GetNumBits(void)
{
    return numBits;
}

uint64_t
BitMap::GetNumBitsSet(void)
{
    return numBitsSet;
}

bool
BitMap::SetNumBitsSet(uint64_t numBits)
{
    numBitsSet = numBits;
    return true;
}

uint64_t*
BitMap::GetMapAddr(void)
{
    return map;
}

bool
BitMap::SetBit(uint64_t bitOffset)
{
    if (unlikely(IsValidBit(bitOffset) == false))
    {
        return false;
    }

    if (IsSetBit(bitOffset) == true)
    {
        return true;
    }

    uint64_t row = bitOffset / BITMAP_ENTRY_BITS;
    uint64_t col = bitOffset % BITMAP_ENTRY_BITS;
    map[row] |= (1ULL << col);
    ++numBitsSet;
    lastSetPosition = bitOffset;
    return true;
}

bool
BitMap::ClearBit(uint64_t bitOffset)
{
    if (unlikely(IsValidBit(bitOffset) == false))
    {
        return false;
    }

    if (IsSetBit(bitOffset) == false)
    {
        return true;
    }

    uint64_t row = bitOffset / BITMAP_ENTRY_BITS;
    uint64_t col = bitOffset % BITMAP_ENTRY_BITS;
    map[row] &= (~(1ULL << col));
    --numBitsSet;
    return true;
}

bool
BitMap::ClearBits(uint64_t begin, uint64_t end)
{
    if (unlikely(IsValidBit(begin) == false || IsValidBit(end) == false))
    {
        return false;
    }
    uint64_t startRow = begin / BITMAP_ENTRY_BITS;
    uint64_t endRow = end / BITMAP_ENTRY_BITS;
    uint64_t startCol, endCol, numClearBits;

    for (uint64_t row = startRow; row <= endRow; ++row)
    {
        if (row == startRow)
        {
            startCol = begin % BITMAP_ENTRY_BITS;
        }
        else
        {
            startCol = 0;
        }

        if (row == endRow)
        {
            endCol = end % BITMAP_ENTRY_BITS;
        }
        else
        {
            endCol = BITMAP_ENTRY_BITS - 1;
        }

        numClearBits = endCol - startCol + 1;
        uint64_t mask = _GetMask(numClearBits, startCol);
        map[row] &= (~mask);
        numBitsSet -= numClearBits;
    }

    return true;
}

void
BitMap::ResetBitmap(void)
{
    for (uint64_t row = 0; row < numEntry; ++row)
    {
        map[row] = 0;
    }
    numBitsSet = 0;
}

void
BitMap::FlipBit(uint64_t bit)
{
    uint64_t row = bit / BITMAP_ENTRY_BITS;
    uint64_t col = bit % BITMAP_ENTRY_BITS;
    bool IsSet = (map[row] & (1ULL << col)) != 0;

    if (IsSet)
    {
        map[row] &= (~(1ULL << col));
    }
    else
    {
        map[row] |= (1ULL << col);
    }
}

uint64_t
BitMap::FindFirstZero(void)
{
    uint64_t row = 0;
    uint64_t offset = 0;

    while (0 == (offset = ffzl(map[row])))
    {
        ++row;
        if (unlikely(row >= numEntry))
        {
            return numBits;
        }
    }

    offset += (row * BITMAP_ENTRY_BITS) - 1;
    if (likely(IsValidBit(offset)))
    {
        return offset;
    }
    else
    {
        return numBits;
    }
}

uint64_t
BitMap::FindFirstSet(uint64_t begin)
{
    if (unlikely(IsValidBit(begin) == false))
    {
        return numBits;
    }

    uint32_t row = begin / BITMAP_ENTRY_BITS;
    uint32_t col = begin % BITMAP_ENTRY_BITS;
    uint64_t begin_val;
    uint32_t offset = 0;

    begin_val = map[row];
    begin_val &= ~((1ULL << col) - 1); // Clear bits under begin

    if (!(offset = ffsl(begin_val)))
    {
        do
        {
            if (++row == numEntry)
            {
                offset = BITMAP_ENTRY_BITS;
                break;
            }
        } while (!(offset = ffsl(map[row])));
    }

    offset += (row * BITMAP_ENTRY_BITS) - 1;
    if (likely(IsValidBit(offset)))
    {
        return offset;
    }
    else
    {
        return numBits;
    }
}

uint64_t
BitMap::FindNextZero(void)
{
    uint64_t posToBegin = (lastSetPosition + 1) % numBits;

    uint64_t foundPos = FindFirstZero(posToBegin);
    if (foundPos == numBits)
    {
        if (unlikely(numBitsSet >= numBits))
        {
            return numBits;
        }
        else
        {
            return FindFirstZero(0);
        }
    }
    else
    {
        return foundPos;
    }
}

uint64_t
BitMap::FindFirstZero(uint64_t begin)
{
    if (unlikely(IsValidBit(begin) == false))
    {
        return numBits;
    }

    uint64_t row = begin / BITMAP_ENTRY_BITS;
    uint64_t col = begin % BITMAP_ENTRY_BITS;
    uint64_t offset = 0;

    uint64_t val = map[row];
    val |= ((1ULL << col) - 1);

    if (0 == (offset = ffzl(val)))
    {
        while (0 == (offset = ffzl(map[++row])))
        {
            if (unlikely(row >= numEntry))
            {
                return numBits;
            }
        }
    }
    offset += (row * BITMAP_ENTRY_BITS) - 1;

    if (likely(IsValidBit(offset)))
    {
        return offset;
    }
    else
    {
        return numBits;
    }
}

uint64_t
BitMap::FindFirstZero(uint64_t begin, uint64_t end)
{
    if (unlikely(IsValidBit(begin) == false || IsValidBit(end) == false))
    {
        return numBits;
    }

    uint64_t row = begin / BITMAP_ENTRY_BITS;
    uint64_t col = begin % BITMAP_ENTRY_BITS;
    uint64_t offset = 0;

    uint64_t val = map[row];
    val |= ((1ULL << col) - 1);

    if (0 == (offset = ffzl(val)))
    {
        while (0 == (offset = ffzl(map[++row])))
        {
            if (unlikely(row >= numEntry))
            {
                return numBits;
            }
        }
    }
    offset += (row * BITMAP_ENTRY_BITS) - 1;

    if (unlikely(!IsValidBit(offset) || (offset > end)))
    {
        return numBits;
    }
    else
    {
        return offset;
    }
}

bool
BitMap::IsValidBit(uint64_t bitOffset)
{
    return bitOffset < numBits;
}

uint64_t
BitMap::GetNumEntry(void)
{
    return numEntry;
}

bool
BitMap::Set(BitMap& inputBitMap)
{
    if (numEntry != inputBitMap.numEntry)
    {
        return false;
    }

    for (uint32_t row = 0; row < numEntry; ++row)
    {
        map[row] |= inputBitMap.map[row];
    }
    return true;
}

BitMap::BitMap(uint64_t totalBits)
{
    assert(totalBits != 0);
    numBitsSet = 0;
    numBits = totalBits;
    lastSetPosition = numBits - 1;

    uint64_t row = totalBits / BITMAP_ENTRY_BITS;
    uint64_t col = totalBits % BITMAP_ENTRY_BITS;
    if (col > 0)
    {
        numEntry = row + 1;
    }
    else
    {
        numEntry = row;
    }

    map = new uint64_t[numEntry]();
    assert(map != nullptr);
}

BitMap::~BitMap(void)
{
    delete[] map;
    map = nullptr;
}

BitMapMutex::BitMapMutex(BitMap* bitmap)
: bitMap(bitmap)
{
}

BitMapMutex::BitMapMutex(uint64_t totalBits)
: BitMapMutex(new BitMap(totalBits))
{
}

BitMapMutex::~BitMapMutex(void)
{
    delete bitMap;
    bitMap = nullptr;
}

bool
BitMapMutex::IsValidBit(uint64_t bitOffset)
{
    return bitMap->IsValidBit(bitOffset);
}

uint64_t
BitMapMutex::FindFirstSetBit(uint64_t begin)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->FindFirstSet(begin);
}

uint64_t
BitMapMutex::SetFirstZeroBit(void)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    uint64_t bit = bitMap->FindFirstZero();
    bitMap->SetBit(bit);
    return bit;
}

uint64_t
BitMapMutex::SetFirstZeroBit(uint64_t begin)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    uint64_t bit = bitMap->FindFirstZero(begin);
    bitMap->SetBit(bit);
    return bit;
}

uint64_t
BitMapMutex::SetFirstZeroBit(uint64_t begin, uint64_t end)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    uint32_t bit = bitMap->FindFirstZero(begin, end);
    bitMap->SetBit(bit);
    return bit;
}

uint64_t
BitMapMutex::SetNextZeroBit(void)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    uint32_t bit = bitMap->FindNextZero();
    bitMap->SetBit(bit);
    return bit;
}

bool
BitMapMutex::SetBit(uint64_t bit)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->SetBit(bit);
}

bool
BitMapMutex::ClearBit(uint64_t bit)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->ClearBit(bit);
}

bool
BitMapMutex::ClearBits(uint64_t begin, uint64_t end)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->ClearBits(begin, end);
}

void
BitMapMutex::ResetBitmap(void)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    bitMap->ResetBitmap();
}

void
BitMapMutex::SetNumBitsSet(uint64_t numBits)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    bitMap->SetNumBitsSet(numBits);
}

uint64_t*
BitMapMutex::GetMapAddr(void)
{
    return bitMap->GetMapAddr();
}

uint64_t
BitMapMutex::GetNumBits(void)
{
    return bitMap->GetNumBits();
}

bool
BitMapMutex::IsSetBit(uint64_t bit)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->IsSetBit(bit);
}

uint64_t
BitMapMutex::GetNumEntry(void)
{
    return bitMap->GetNumEntry();
}

void
BitMapMutex::FlipBit(uint64_t bit)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    bitMap->FlipBit(bit);
}
void
BitMapMutex::PrintMap(void)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    bitMap->PrintMap();
}

uint64_t
BitMapMutex::GetNumBitsSet(void)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->GetNumBitsSet();
}

uint64_t
BitMapMutex::GetNumBitsSetWoLock(void)
{
    return bitMap->GetNumBitsSet();
}

uint64_t
BitMapMutex::GetNumBitsSet(uint64_t begin, uint64_t end)
{
    assert(end <= bitMap->GetNumBits());

    std::unique_lock<std::mutex> lock(bitMapLock);
    uint64_t ret = 0;

    for (uint64_t bitOffset = begin; bitOffset <= end; ++bitOffset)
    {
        if (bitMap->IsSetBit(bitOffset))
        {
            ++ret;
        }
    }
    return ret;
}

bool
BitMapMutex::SetBitMap(BitMapMutex& inputBitMapMutex)
{
    std::unique_lock<std::mutex> lock(bitMapLock);
    return bitMap->Set(*inputBitMapMutex.bitMap);
}

std::mutex&
BitMapMutex::GetLock(void)
{
    return bitMapLock;
}

} // namespace pos
