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

#include "mfs_io_range_overlap_chker.h"
#include "src/include/branch_prediction.h"

namespace pos
{
MetaFsIoRangeOverlapChker::MetaFsIoRangeOverlapChker(void)
: outstandingIoMap(nullptr)
{
}

MetaFsIoRangeOverlapChker::MetaFsIoRangeOverlapChker(BitMap* outstandingIoMap)
: outstandingIoMap(outstandingIoMap)
{
}

MetaFsIoRangeOverlapChker::~MetaFsIoRangeOverlapChker(void)
{
    if (nullptr != outstandingIoMap)
    {
        delete outstandingIoMap;
    }
}

void
MetaFsIoRangeOverlapChker::Init(MetaLpnType maxLpn)
{
    outstandingIoMap = new BitMap(maxLpn + 1);
    outstandingIoMap->ResetBitmap();
}

bool
MetaFsIoRangeOverlapChker::IsRangeOverlapConflicted(MetaFsIoRequest* newReq)
{
    if (unlikely(outstandingIoMap == nullptr))
        return false;

    if (false == outstandingIoMap->IsSetBit(newReq->baseMetaLpn))
        return false;

    return true;
}

void
MetaFsIoRangeOverlapChker::FreeLockContext(uint64_t startLpn, bool isRead)
{
    if (unlikely(outstandingIoMap == nullptr))
        return;

    if (true != isRead)
    {
        if (!outstandingIoMap->IsSetBit(startLpn))
        {
            POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                "The bit is already cleared, startLpn: {}", startLpn);
            assert(false);
        }
        outstandingIoMap->ClearBit(startLpn);
    }
}

void
MetaFsIoRangeOverlapChker::PushReqToRangeLockMap(MetaFsIoRequest* newReq)
{
    if (likely(outstandingIoMap != nullptr))
    {
        if (newReq->reqType == MetaIoRequestType::Write)
        {
            if (outstandingIoMap->IsSetBit(newReq->baseMetaLpn))
            {
                POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
                    "The bit is already set, baseMetaLpn: {}", newReq->baseMetaLpn);
                assert(false);
            }
            outstandingIoMap->SetBit(newReq->baseMetaLpn);
        }
    }
}

BitMap*
MetaFsIoRangeOverlapChker::GetOutstandingMioMap(void)
{
    return outstandingIoMap;
}

uint64_t
MetaFsIoRangeOverlapChker::GetOutstandingMioCount(void)
{
    return outstandingIoMap->GetNumBitsSet();
}
} // namespace pos
