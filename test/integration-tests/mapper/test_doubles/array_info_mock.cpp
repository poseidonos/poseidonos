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

#include "src/include/array_state_type.h"
#include "test/integration-tests/mapper/test_doubles/array_info_mock.h"

#include <string>

namespace pos
{

MockArrayInfo::MockArrayInfo(void)
{
    pls = new PartitionLogicalSize();
#if 0
    pls->blksPerChunk = 1;
    pls->blksPerStripe = 10;
    pls->stripesPerSegment = 1;
    pls->totalSegments = 1;
    pls->totalStripes = 3;
    pls->chunksPerStripe = 10;
    pls->minWriteBlkCnt = 1;
#else
    pls->totalStripes = 131072;
    pls->blksPerStripe = 512;
    pls->totalSegments = 128;
    pls->stripesPerSegment = 1024;
#endif
}

MockArrayInfo::~MockArrayInfo(void)
{
    delete pls;
}

const PartitionLogicalSize*
MockArrayInfo::GetSizeInfo(PartitionType type)
{
    return pls;
}

DeviceSet<string>
MockArrayInfo::GetDevNames(void)
{
    DeviceSet<string> ds;
    return ds;
}

string
MockArrayInfo::GetName(void)
{
    std::string str = "";
    return str;
}

unsigned int
MockArrayInfo::GetIndex(void)
{
    unsigned int idx = 0;
    return idx;
}

string
MockArrayInfo::GetMetaRaidType(void)
{
    std::string str = "";
    return str;
}

string
MockArrayInfo::GetDataRaidType(void)
{
    std::string str = "";
    return str;
}

id_t 
MockArrayInfo::GetUniqueId(void)
{
    return 0;
}

ArrayStateType
MockArrayInfo::GetState(void)
{
    ArrayStateType ast;
    return ast;
}

StateContext*
MockArrayInfo::GetStateCtx(void)
{
    return nullptr;
}

uint32_t
MockArrayInfo::GetRebuildingProgress(void)
{
    return 0;
}

}   // namespace pos
