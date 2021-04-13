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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#ifndef METHOD_H_
#define METHOD_H_

#include <list>

#include "src/include/address_type.h"
#include "src/include/recover_func.h"
#include "src/include/raid_type.h"

using namespace std;

namespace pos
{
struct FtSizeInfo
{
    uint32_t minWriteBlkCnt;
    uint32_t backupBlkCnt;
    uint32_t blksPerChunk;
    uint32_t blksPerStripe;
    uint32_t chunksPerStripe;
};

class Method
{
public:
    Method()
    {
    }
    virtual ~Method()
    {
    }

    RecoverFunc& GetRecoverFunc(void);
    const FtSizeInfo* GetSizeInfo(void);

    virtual int Translate(FtBlkAddr&, const LogicalBlkAddr&) = 0;
    virtual int Convert(list<FtWriteEntry>&, const LogicalWriteEntry&) = 0;
    virtual list<FtBlkAddr> GetRebuildGroup(FtBlkAddr fba) = 0;
    RaidType GetRaidType(void);

protected:
    virtual void _BindRecoverFunc(void) = 0;

    FtSizeInfo ftSize_ = {
        0,
    };
    RecoverFunc recoverFunc_ = nullptr;
    RaidType raidType;
};

} // namespace pos
#endif // METHOD_H_
