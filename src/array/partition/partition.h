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

#ifndef PARTITION_H_
#define PARTITION_H_

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/method.h"
#include "src/array/ft/rebuild_behavior.h"
#include "src/array/partition/partition_size_info.h"
#include "src/include/address_type.h"

using namespace std;

namespace ibofos
{
class UBlockDevice;
class ArrayDevice;
class Ubio;

enum PartitionType
{
    META_NVM = 0,
    WRITE_BUFFER,
    META_SSD,
    USER_DATA,
    PARTITION_TYPE_MAX
};

class Partition
{
public:
    Partition(PartitionType type,
        PartitionPhysicalSize physicalSize,
        vector<ArrayDevice*> devs,
        Method* method);
    virtual ~Partition();

    virtual int Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src) = 0;
    virtual int Convert(list<PhysicalWriteEntry>& dst,
        const LogicalWriteEntry& src) = 0;
    const PartitionLogicalSize* GetLogicalSize();
    const PartitionPhysicalSize* GetPhysicalSize();
    bool IsValidLba(uint64_t lba);
    int FindDevice(ArrayDevice* dev);
    virtual int Rebuild(RebuildBehavior* behavior) = 0;
    virtual int RebuildRead(UbioSmartPtr targetUbio) = 0;
    bool TryLock(StripeId stripeId);
    void Unlock(StripeId stripeId);
    Method*
    GetMethod()
    {
        return method_;
    }
    virtual void
    Format()
    {
    }

protected:
    PartitionType type_;
    PartitionLogicalSize logicalSize_;
    PartitionPhysicalSize physicalSize_;
    vector<ArrayDevice*> devs_;
    Method* method_ = nullptr;
    bool _IsValidAddress(const LogicalBlkAddr& lsa);
    bool _IsValidEntry(const LogicalWriteEntry& entry);
};

} // namespace ibofos
#endif // PARTITION_H_
