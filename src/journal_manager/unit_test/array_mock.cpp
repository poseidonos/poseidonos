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

#include "array_mock.h"

#include "test_info.h"

MockArray::MockArray(TestInfo* _testInfo)
{
    testInfo = _testInfo;
    ON_CALL(*this, GetSizeInfo).WillByDefault(::testing::Invoke(this, &MockArray::_GetSizeInfo));

    userSizeInfo = new PartitionLogicalSize();
}

MockArray::~MockArray(void)
{
    delete userSizeInfo;
}

const PartitionLogicalSize*
MockArray::_GetSizeInfo(PartitionType type)
{
    if (type == PartitionType::USER_DATA)
    {
        userSizeInfo->minWriteBlkCnt = 0;
        userSizeInfo->blksPerChunk = testInfo->numBlksPerChunk;
        userSizeInfo->blksPerStripe = testInfo->numBlksPerStripe;
        userSizeInfo->chunksPerStripe = testInfo->numChunksPerStripe;
        userSizeInfo->stripesPerSegment = testInfo->numStripesPerSegment;
        userSizeInfo->totalSegments = testInfo->numUserSegments;
        userSizeInfo->totalStripes = testInfo->numUserStripes;
        return userSizeInfo;
    }
    else
    {
        // TODO(huijeong.kim): add wb size info if required
        return nullptr;
    }
}
