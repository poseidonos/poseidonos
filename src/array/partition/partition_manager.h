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

#ifndef PARTITION_MANAGER_H_
#define PARTITION_MANAGER_H_

#include <array>
#include <string>
#include <vector>

#include "partition.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/cpu_affinity/affinity_manager.h"
using namespace std;

namespace pos
{
class Ubio;
class ArrayInterface;
class IAbrControl;

class PartitionManager
{
    friend class ParityLocationWbtCommand;

public:
    PartitionManager(
        string array,
        IAbrControl* abr,
        AffinityManager* affinityManager = AffinityManagerSingleton::Instance(),
        IODispatcher* ioDispatcher = IODispatcherSingleton::Instance());
    virtual ~PartitionManager();
    virtual const PartitionLogicalSize* GetSizeInfo(PartitionType type);
    virtual int CreateAll(vector<ArrayDevice*> buf, vector<ArrayDevice*> data,
        ArrayInterface* intf, uint32_t arrayIndex);
    virtual void DeleteAll(ArrayInterface* intf);
    virtual void FormatMetaPartition(vector<ArrayDevice*> data, ArrayInterface* intf, uint32_t arrayIndex);

private:
    int _CreateMetaSsd(vector<ArrayDevice*> devs, ArrayInterface* intf, uint32_t arrayIndex);
    int _CreateUserData(const vector<ArrayDevice*> devs, ArrayDevice* nvm, ArrayInterface* intf, uint32_t arrayIndex);
    int _CreateMetaNvm(ArrayDevice* dev, ArrayInterface* intf, uint32_t arrayIndex);
    int _CreateWriteBuffer(ArrayDevice* dev, ArrayInterface* intf, uint32_t arrayIndex);
    ArrayDevice* _GetBaseline(const vector<ArrayDevice*>& devs);

    string arrayName_ = "";
    array<Partition*, PartitionType::PARTITION_TYPE_MAX> partitions_;
    IAbrControl* abrControl = nullptr;
    AffinityManager* affinityManager = nullptr;
    IODispatcher* ioDispatcher = nullptr;
};

} // namespace pos
#endif // PARTITION_MANAGER_H_
