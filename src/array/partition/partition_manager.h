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
#include <condition_variable>
#include <list>
#include <mutex>
#include <vector>

#include "partition.h"
#include "rebuild_job.h"
#include "src/include/address_type.h"

using namespace std;

namespace ibofos
{
using RebuildResultCallback = function<void(RebuildState)>;
class Ubio;

struct RebuildJobs
{
    list<RebuildJob*> jobList;
    RebuildState state;
    RebuildProgress* progress = nullptr;
    RebuildResultCallback resultCb = nullptr;
};

class PartitionManager
{
    friend class WbtCmdHandler;

public:
    PartitionManager();
    virtual ~PartitionManager()
    {
    }
    const PartitionLogicalSize* GetSizeInfo(PartitionType type);
    int Translate(const PartitionType type,
        PhysicalBlkAddr& dst,
        const LogicalBlkAddr& src);
    int Convert(const PartitionType type,
        list<PhysicalWriteEntry>& dst,
        const LogicalWriteEntry& src);
    int CreateAll(ArrayDevice* nvm, vector<ArrayDevice*> dataDevs);
    void DeleteAll();
    void Rebuild(ArrayDevice* target, RebuildResultCallback cb);
    void StopRebuilding();
    uint32_t GetRebuildingProgress();
    int RebuildRead(UbioSmartPtr ubio);
    bool TryLock(PartitionType type, StripeId stripeId);
    void Unlock(PartitionType type, StripeId stripeId);

private:
    int _CreateMetaSsd(vector<ArrayDevice*> devs);
    int _CreateUserData(const vector<ArrayDevice*> devs, const ArrayDevice* nvm);
    int _CreateMetaNvm(ArrayDevice* dev);
    int _CreateWriteBuffer(ArrayDevice* dev);
    void _DoRebuilding(RebuildJob* job);
    void _RebuildDone(void);
    void _RebuildingProgressUpdated(void);
    void _WaitRebuildingDone(void);
    ArrayDevice* _GetBaseline(const vector<ArrayDevice*>& devs);

    array<Partition*, PartitionType::PARTITION_TYPE_MAX> partitions_;
    mutex rebuildMtx;
    condition_variable rebuildCv;
    RebuildJobs rebuildJobs;
};

} // namespace ibofos
#endif // PARTITION_MANAGER_H_
