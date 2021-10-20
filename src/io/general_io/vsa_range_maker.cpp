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

#include "src/io/general_io/vsa_range_maker.h"
#include "src/mapper/include/mapper_const.h"
#include "src/mapper_service/mapper_service.h"

namespace pos
{
VsaRangeMaker::VsaRangeMaker(uint32_t volumeId, BlkAddr startRba,
    uint32_t blockCount, int arrayId)
: VsaRangeMaker(volumeId, startRba, blockCount, MapperServiceSingleton::Instance()->GetIVSAMap(arrayId) )
{
}

VsaRangeMaker::VsaRangeMaker(uint32_t volumeId, BlkAddr startRba,
    uint32_t blockCount, IVSAMap* iVSAMap)
: vsaRange({.startVsa = {.stripeId = 0, .offset = 0}, .numBlks = 0}),
  retry(false), iVSAMap(iVSAMap)
{
    VsaArray vsaArray;

    int ret = iVSAMap->GetVSAs(volumeId, startRba, blockCount, vsaArray);

    if (ret < 0)
    {
        throw ret;
    }

    for (uint32_t cnt = 0; cnt < blockCount; ++cnt)
    {
        VirtualBlkAddr vsa = vsaArray[cnt];
        bool overWrite = (IsUnMapVsa(vsa) == false);
        if (overWrite)
        {
            if (vsaRange.numBlks == 0)
            {
                _SetNewRange(vsa);
            }
            else if (_IsContiguous(vsa))
            {
                _Add();
            }
            else
            {
                _Cut();
                _SetNewRange(vsa);
            }
        }
    }

    _Cut();
}

void
VsaRangeMaker::_Add(void)
{
    vsaRange.numBlks++;
}

void
VsaRangeMaker::_SetNewRange(VirtualBlkAddr& vsa)
{
    vsaRange.startVsa = vsa;
    vsaRange.numBlks = 1;
}

bool
VsaRangeMaker::_IsContiguous(VirtualBlkAddr& vsa)
{
    bool isSameStripe = (vsaRange.startVsa.stripeId == vsa.stripeId);
    bool isContiguous =
        (vsaRange.startVsa.offset + vsaRange.numBlks == vsa.offset);

    return isSameStripe && isContiguous;
}

void
VsaRangeMaker::_Cut(void)
{
    if (vsaRange.numBlks > 0)
    {
        vsaRangeVector.push_back(vsaRange);
    }
}

VsaRangeMaker::~VsaRangeMaker(void)
{
}

uint32_t
VsaRangeMaker::GetCount(void)
{
    return vsaRangeVector.size();
}

VirtualBlks&
VsaRangeMaker::GetVsaRange(uint32_t index)
{
    return vsaRangeVector[index];
}

} // namespace pos
