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

#include "test_info.h"

TestInfo::TestInfo(void)
{
    numWbStripes = 64;
    numStripesPerSegment = 64;
    numUserSegments = 64;
    numUserStripes = numStripesPerSegment * numUserSegments;

    numBlksPerChunk = 32;
    numChunksPerStripe = 4;
    numBlksPerStripe = numBlksPerChunk * numChunksPerStripe;
    numUserBlocks = numUserStripes * numBlksPerStripe;

    numVolumeMap = 256;
    numMap = numVolumeMap + 1;
    defaultTestVol = 1;
    maxNumVolume = 5;
    numTest = 1000;

    maxVolumeSizeInBlock = numUserBlocks / maxNumVolume;
    defaultTestVolSizeInBlock = maxVolumeSizeInBlock; // 1GB
    metaPageSize = 4032;
}

void
TestInfo::SetNumStripesPerSegment(int value)
{
    numStripesPerSegment = value;
    numUserStripes = numStripesPerSegment * numUserSegments;
}
