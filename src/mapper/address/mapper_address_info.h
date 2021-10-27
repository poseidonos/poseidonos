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

#include <cstdint>
#include <string>

namespace pos
{
class MapperAddressInfo
{
public:
    MapperAddressInfo(void) = default;
    explicit MapperAddressInfo(IArrayInfo* iArrayInfo_);
    virtual ~MapperAddressInfo(void);
    virtual void SetupAddressInfo(int mpageSize_);
    virtual std::string GetArrayName(void);
    virtual int GetArrayId(void);
    virtual bool IsUT(void) { return isUT; }

    virtual uint32_t GetBlksPerStripe(void) { return blksPerStripe; }
    virtual uint32_t GetNumWbStripes(void) { return numWbStripes; }
    virtual uint32_t GetMaxVSID(void) { return maxVsid; }
    virtual uint64_t GetMpageSize(void);

    void SetMaxVSID(uint32_t cnt) { maxVsid = cnt; }
    void SetBlksPerStripe(uint32_t cnt) { blksPerStripe = cnt; }
    void SetNumWbStripes(uint32_t cnt) { numWbStripes = cnt; }
    void SetMPageSize(uint32_t cnt) { mpageSize = cnt; }
    void SetIsUT(bool ut) { isUT = ut; }

private:
    uint32_t maxVsid;
    uint32_t blksPerStripe;
    uint32_t numWbStripes;
    uint32_t mpageSize;
    IArrayInfo* iArrayInfo;
    bool isUT;
};

} // namespace pos
