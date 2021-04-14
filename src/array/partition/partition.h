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

#include <string>
#include <vector>

#include "src/include/partition_type.h"
#include "src/include/address_type.h"
#include "src/array/device/array_device.h"
#include "src/array/ft/method.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/array/service/io_translator/i_translator.h"

using namespace std;

namespace pos
{
class Ubio;

class Partition : public ITranslator
{
public:
    Partition(
        string array,
        PartitionType type,
        PartitionPhysicalSize physicalSize,
        vector<ArrayDevice*> devs,
        Method* method);
    virtual ~Partition(void);

    int Create(PartitionPhysicalSize size, vector<ArrayDevice*> devs);
    const PartitionLogicalSize* GetLogicalSize();
    const PartitionPhysicalSize* GetPhysicalSize();
    bool IsValidLba(uint64_t lba);
    int FindDevice(ArrayDevice* dev);
    Method* GetMethod(void);
    virtual void Format(void) {}

protected:
    bool _IsValidAddress(const LogicalBlkAddr& lsa);
    bool _IsValidEntry(const LogicalWriteEntry& entry);
    string arrayName_;
    PartitionType type_;
    PartitionLogicalSize logicalSize_;
    PartitionPhysicalSize physicalSize_;
    vector<ArrayDevice*> devs_;
    Method* method_ = nullptr;
    uint64_t lastLba_ = 0;
};

} // namespace pos
#endif // PARTITION_H_
