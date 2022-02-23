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

#include <mutex>

#define BITMAP_ENTRY_BITS (64) // entry size of map_ in bits
#define BITMAP_ENTRY_SIZE (8)  // entry size of map_ in bytes

namespace pos
{
class BitMap
{
public:
    explicit BitMap(uint64_t totalBits);
    virtual ~BitMap(void);

    virtual uint64_t GetNumBits(void);
    virtual uint64_t GetNumBitsSet(void);
    bool SetNumBitsSet(uint64_t numBits);
    uint64_t GetNumEntry(void);
    uint64_t* GetMapAddr(void);

    void PrintMap(void);
    virtual bool SetBit(uint64_t bitOffset);
    virtual bool ClearBit(uint64_t bitOffset);
    virtual bool ClearBits(uint64_t begin, uint64_t end);
    virtual void ResetBitmap(void);
    virtual bool IsSetBit(uint64_t bit);
    virtual void FlipBit(uint64_t bit);
    virtual uint64_t FindFirstSet(uint64_t begin);
    virtual uint64_t FindFirstZero(void);
    virtual uint64_t FindFirstZero(uint64_t begin);
    virtual uint64_t FindFirstZero(uint64_t begin, uint64_t end);
    virtual uint64_t FindNextZero(void);
    virtual bool IsValidBit(uint64_t bitOffset);
    virtual bool Set(BitMap &inputBitMap);

private:
    uint64_t _GetMask(uint64_t numSetBits, uint64_t offset);

    uint64_t* map;
    uint64_t numBits; // The total number of bits set by the Ctor
    uint64_t numEntry;
    uint64_t numBitsSet;
    uint64_t lastSetPosition;
};

class BitMapMutex
{
public:
    explicit BitMapMutex(BitMap* bitmap);       // Ctor for DoCs Injection in UT
    explicit BitMapMutex(uint64_t totalBits);   // Ctor for Production code
    virtual ~BitMapMutex(void);

    virtual bool IsValidBit(uint64_t bitOffset);
    virtual uint64_t FindFirstSetBit(uint64_t begin);
    uint64_t SetFirstZeroBit(void);
    virtual uint64_t SetFirstZeroBit(uint64_t begin);
    uint64_t SetFirstZeroBit(uint64_t begin, uint64_t end);
    virtual uint64_t SetNextZeroBit(void);
    virtual bool SetBit(uint64_t bit);
    virtual bool ClearBit(uint64_t bit);
    virtual bool ClearBits(uint64_t begin, uint64_t end);
    virtual void ResetBitmap(void);
    virtual bool IsSetBit(uint64_t bit);
    virtual void FlipBit(uint64_t bit);
    virtual uint64_t GetNumEntry(void);
    virtual uint64_t GetNumBits(void);
    virtual void SetNumBitsSet(uint64_t numBits);
    virtual uint64_t* GetMapAddr(void);
    void PrintMap(void);
    virtual uint64_t GetNumBitsSet(void);
    virtual uint64_t GetNumBitsSetWoLock(void);
    uint64_t GetNumBitsSet(uint64_t begin, uint64_t end);
    bool SetBitMap(BitMapMutex& inputBitMapMutex);
    virtual std::mutex& GetLock(void);

private:
    std::mutex bitMapLock;
    // DoCs
    BitMap* bitMap;
};

} // namespace pos
