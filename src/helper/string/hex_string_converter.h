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

#include "stdlib.h"
#include <cstdint>

using namespace std;

inline uint32_t
hex_to_uint32(char* src, uint32_t len)
{
    uint32_t val = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        val += (uint8_t)src[i] << (8  * (len - 1 - i));
    }
    return val;
}

inline void
uint32_to_hex(uint32_t value, char* dest, uint32_t len)
{
    uint32_t copiedValue = value;

    for (uint32_t i = len - 1; i >= 0; i--)
    {
        dest[i] = (copiedValue & 255);
        copiedValue >>= 8;
        if (copiedValue <= 0)
        {
            break;
        }
    }
}

inline uint64_t
hex_to_uint64(char* src, uint32_t len)
{
    uint64_t val = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        val += (uint8_t)src[i] << (8  * (len - 1 - i));
    }
    return val;
}

inline void
uint64_to_hex(uint64_t value, char* dest, uint32_t len)
{
    uint64_t copiedValue = value;

    for (uint32_t i = len - 1; i >= 0; i--)
    {
        dest[i] = (copiedValue & 255);
        copiedValue >>= 8;
        if (copiedValue <= 0)
        {
            break;
        }
    }
}
