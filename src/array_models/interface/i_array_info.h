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

#include <string>

#include "src/array_models/dto/device_set.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/include/array_state_type.h"
#include "src/include/partition_type.h"
#include "src/state/state_context.h"

#include "src/array/device/i_array_device_manager.h"
using namespace std;

namespace pos
{
class IArrayInfo
{
public:
    virtual const PartitionLogicalSize* GetSizeInfo(PartitionType type) = 0;
    virtual DeviceSet<string> GetDevNames(void) = 0;
    virtual string GetName(void) = 0;
    virtual unsigned int GetIndex(void) = 0;
    virtual string GetMetaRaidType(void) = 0;
    virtual string GetDataRaidType(void) = 0;
    virtual string GetCreateDatetime(void) = 0;
    virtual string GetUpdateDatetime(void) = 0;
    virtual id_t GetUniqueId(void) = 0;
    virtual ArrayStateType GetState(void) = 0;
    virtual StateContext* GetStateCtx(void) = 0;
    virtual uint32_t GetRebuildingProgress(void) = 0;
    virtual IArrayDevMgr* GetArrayManager(void) = 0;
    virtual bool IsWriteThroughEnabled(void) = 0;
};
} // namespace pos
