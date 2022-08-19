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

#include "src/array/device/array_device.h"
#include "stripe_locker.h"
#include "i_io_locker.h"
#include "locker_group.h"
#include "src/lib/singleton.h"
#include "src/include/partition_type.h"

#include <string>
#include <map>
#include <vector>
#include <set>
using namespace std;

namespace pos
{
class IOLocker : public IIOLocker
{
public:
    explicit IOLocker(string lockerName);
    virtual ~IOLocker(void) {}
    bool Register(vector<ArrayDevice*> devList);
    void Unregister(vector<ArrayDevice*> devList);
    bool TryBusyLock(IArrayDevice* dev, StripeId from, StripeId to) override;
    bool TryBusyLock(set<IArrayDevice*>& devs, StripeId from, StripeId to, IArrayDevice*& failed) override;
    bool ResetBusyLock(IArrayDevice* dev, bool forceReset = false) override;
    bool TryLock(set<IArrayDevice*>& devs, StripeId val) override;
    void Unlock(IArrayDevice* dev, StripeId val) override;
    void Unlock(set<IArrayDevice*>& devs, StripeId val) override;
    void WriteBusyLog(IArrayDevice* dev) override;

private:
    StripeLocker* _Find(IArrayDevice* dev);
    map<IArrayDevice*, StripeLocker*> lockers;
    LockerGroup group;
    string name;
};
} // namespace pos
