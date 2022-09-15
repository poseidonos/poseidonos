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

#include "src/array/rebuild/rebuild_method.h"
#include "src/include/recover_func.h"
#include "src/include/i_array_device.h"

#include <vector>

using namespace std;

namespace pos
{
class NToMRebuild : public RebuildMethod
{
public:
    explicit NToMRebuild(vector<IArrayDevice*> src, vector<IArrayDevice*> dst, RecoverFunc recoverFunc);
    virtual ~NToMRebuild();
    void SetBackupMethod(NToMRebuild* backup);
    void SetFailOver(void);
    bool IsFailOver(void);
    int Recover(int arrayIndex, StripeId stripeId, const PartitionPhysicalSize* pSize, StripeRebuildDoneCallback callback) override;

private:
    void _Read(int arrayIndex, StripeId stripeId, const PartitionPhysicalSize* pSize, StripeRebuildDoneCallback callback);
    void _ReadDone(int arrayIndex, StripeId stripeId, const PartitionPhysicalSize* pSize, StripeRebuildDoneCallback callback, void* src, int readResult);
    void _Recover(int arrayIndex, StripeId stripeId, const PartitionPhysicalSize* pSize, StripeRebuildDoneCallback callback, void* src);
    void _RecoverDone(int arrayIndex, StripeId stripeId, const PartitionPhysicalSize* pSize, StripeRebuildDoneCallback callback, void* mem, int recoverResult);
    void _Write(int arrayIndex, StripeId stripeId, const PartitionPhysicalSize* pSize, StripeRebuildDoneCallback callback, void* mem);
    void _WriteDone(int arrayIndex, StripeId stripeId, StripeRebuildDoneCallback callback, void* mem, int writeResult);
    vector<IArrayDevice*> src;
    vector<IArrayDevice*> dst;
    RecoverFunc recoverFunc = nullptr;
    uint64_t airKey = 0;
    NToMRebuild* backupMethod = nullptr;
    bool isFailOver = false;
};
} // namespace pos
