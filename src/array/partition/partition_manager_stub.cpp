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

#include "partition_manager.h"

namespace ibofos
{
PartitionManager::PartitionManager()
{
}

const PartitionLogicalSize*
PartitionManager::GetSizeInfo(PartitionType type)
{
    return nullptr;
}

int
PartitionManager::CreateAll(ArrayDevice* nvm,
    vector<ArrayDevice*> dataDevs)
{
    return 0;
}

int
PartitionManager::_CreateMetaNvm(ArrayDevice* dev)
{
    return 0;
}

int
PartitionManager::_CreateWriteBuffer(ArrayDevice* dev)
{
    return 0;
}

int
PartitionManager::_CreateMetaSsd(vector<ArrayDevice*> devs)
{
    return 0;
}

int
PartitionManager::_CreateUserData(const vector<ArrayDevice*> devs,
    const ArrayDevice* nvm)
{
    return 0;
}

void
PartitionManager::DeleteAll()
{
}

int
PartitionManager::Translate(const PartitionType type,
    PhysicalBlkAddr& dst,
    const LogicalBlkAddr& src)
{
    return 0;
}

int
PartitionManager::Convert(const PartitionType type,
    list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    return 0;
}

void
PartitionManager::Rebuild(ArrayDevice* target, RebuildResultCallback cb)
{
}

void
PartitionManager::_DoRebuilding(RebuildJob* job)
{
}

void
PartitionManager::StopRebuilding()
{
}

int
PartitionManager::RebuildRead(UbioSmartPtr ubio)
{
    return 0;
}

bool
PartitionManager::TryLock(PartitionType type, StripeId stripeId)
{
    return false;
}

void
PartitionManager::Unlock(PartitionType type, StripeId stripeId)
{
}

ArrayDevice*
PartitionManager::_GetBaseline(const vector<ArrayDevice*>& devs)
{
    return nullptr;
}

void
PartitionManager::_RebuildDone()
{
}

void
PartitionManager::_WaitRebuildingDone()
{
}

uint32_t
PartitionManager::GetRebuildingProgress()
{
    return 0;
}

} // namespace ibofos
