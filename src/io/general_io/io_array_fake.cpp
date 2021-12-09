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


#include <iostream>

#include "io_submit_handler_test.h"
#include "src/array/array.h"
#include "src/array/device/array_device.h"
#include "src/include/partition_type.h"
#include "src/array_models/dto/partition_logical_size.h"

namespace pos
{
static PartitionLogicalSize userDataPartitionLogicalSize;

static ArrayDevice* arrayDevice[IOSubmitHandlerTest::SSD_COUNT];

const int Array::LOCK_ACQUIRE_FAILED = -1;

Array::Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr, IStateControl* iState)
:   state(nullptr),
    svc(nullptr),
    ptnMgr(nullptr),
    devMgr_(nullptr),
    sysDevMgr(nullptr),
    rebuilder(nullptr)
{
    userDataPartitionLogicalSize.blksPerChunk = BLOCKS_IN_CHUNK;
    userDataPartitionLogicalSize.blksPerStripe =
        BLOCKS_IN_CHUNK * IOSubmitHandlerTest::SSD_COUNT;
}

Array::~Array()
{
    for (ArrayDevice* device : arrayDevice)
    {
        if (nullptr != device)
        {
            delete device;
        }
    }
}

const PartitionLogicalSize*
Array::GetSizeInfo(PartitionType type)
{
    const PartitionLogicalSize* sizeInfo = &userDataPartitionLogicalSize;
    return sizeInfo;
}

int
Array::DetachDevice(UblockSharedPtr uBlock)
{
    return 0;
}

} // namespace pos
