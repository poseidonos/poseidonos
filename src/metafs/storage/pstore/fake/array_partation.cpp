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

#include "mfs_io_config.h"
#include "src/array/array_partition.h"

using namespace ibofos;
/**
 * Constructor
 *
 * Setting SizeInfo of meta Data Area on devices.
 * It default for testing purposes.
 */

ArrayPartition::ArrayPartition(PartitionType partition, uint64_t devSize, uint32_t listSize)
{
    sizeInfo.blkSizeByte = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    sizeInfo.dataBlksPerStripe = 64 * NUM_OF_DEVICES;
    sizeInfo.totalStripeCnt = (capacity / sizeInfo.blkSizeByte) / sizeInfo.dataBlksPerStripe;
}

/**
 * Returns deviceLba strcture which provides physical address and device for IO.
 *
 * @lsa logical address information
 *
 * @return physical address and device
 */
DeviceLba
ArrayPartition::TranslateToDeviceLBA(LogicalBlkAddr lsa)
{
    DeviceLba deviceLba;
    deviceLba.dev = NULL;
    deviceLba.lba = lsa.stripeId * sizeInfo.blkSizeByte + lsa.offset; // set random value between 0 and maxSize.
    return deviceLba;
}
