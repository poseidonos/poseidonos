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

#include "src/include/address_type.h"

#include <string>

namespace pos
{

class IMapperWbt
{
public:
    virtual int GetMapLayout(std::string fname) = 0;
    virtual int ReadVsaMap(int volId, std::string fname) = 0;
    virtual int ReadVsaMapEntry(int volId, BlkAddr rba, std::string fname) = 0;
    virtual int WriteVsaMap(int volId, std::string fname) = 0;
    virtual int WriteVsaMapEntry(int volId, BlkAddr rba, VirtualBlkAddr vsa) = 0;
    virtual int ReadStripeMap(std::string fname) = 0;
    virtual int ReadStripeMapEntry(StripeId vsid, std::string fname) = 0;
    virtual int WriteStripeMap(std::string fname) = 0;
    virtual int WriteStripeMapEntry(StripeId vsid, StripeLoc loc, StripeId lsid) = 0;
    virtual int ReadReverseMap(StripeId vsid, std::string fname) = 0;
    virtual int ReadWholeReverseMap(std::string fname) = 0;
    virtual int ReadReverseMapEntry(StripeId vsid, BlkOffset offset, std::string fname) = 0;
    virtual int WriteReverseMap(StripeId vsid, std::string fname) = 0;
    virtual int WriteWholeReverseMap(std::string fname) = 0;
    virtual int WriteReverseMapEntry(StripeId vsid, BlkOffset offset, BlkAddr rba, uint32_t volumeId) = 0;
};

} // namespace pos
