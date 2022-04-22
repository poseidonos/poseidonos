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

#ifndef _INCLUDE_MSS_DISK_PLACE_H_
#define _INCLUDE_MSS_DISK_PLACE_H_

#include <string>

#include "meta_storage_specific.h"
#include "os_header.h"
#include "src/array/array.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/address_type.h"

namespace pos
{
/**
 * class MssDiskPlace is abstract class for getting the physical address
 * From 1.Inplace update
 *      2.OutOfPlace update
 * To decouple mss_on_disk class from where data is stored.
 */
class MssDiskPlace
{
public:
    virtual ~MssDiskPlace(void);
    virtual const pos::PartitionLogicalSize* GetPartitionSizeInfo(void);
    virtual uint64_t GetMetaDiskCapacity(void);
    virtual pos::LogicalBlkAddr CalculateOnDiskAddress(uint64_t metaLpn) = 0;
    virtual uint32_t GetMaxLpnCntPerIOSubmit(void) = 0;
    virtual uint32_t GetLpnCntPerChunk(void) = 0;
    pos::PartitionType
    GetPartitionType(void)
    {
        return partitionType;
    }

protected:
    MetaStorageType mediaType;
    pos::PartitionType partitionType;
    pos::IArrayInfo* array;
};
} // namespace pos

#endif // _INCLUDE_MSS_DISK_PLACE_H_
