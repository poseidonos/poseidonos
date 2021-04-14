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

#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "array_mgmt_policy.h"
#include "src/array_components/array_components.h"
#include "src/array_mgmt/interface/i_array_mgmt.h"
#include "src/device/i_device_event.h"
#include "src/lib/singleton.h"
#include "src/rebuild/array_rebuilder.h"
#include "src/rebuild/interface/i_rebuild_notification.h"

using namespace std;

namespace pos
{
class AbrManager;
struct ArrayBootRecord;
class ArrayManager : public IArrayMgmt, public IDeviceEvent, public IRebuildNotification
{
    friend class FlushAllUserDataWbtCommand;
    friend class GcWbtCommand;
    friend class ParityLocationWbtCommand;

public:
    ArrayManager();
    virtual ~ArrayManager();
    int Create(string name, DeviceSet<string> devs, string raidtype) override;
    int Delete(string name) override;
    int Mount(string name) override;
    int Unmount(string name) override;
    int AddDevice(string name, string dev) override;
    int RemoveDevice(string name, string dev) override;
    int DeviceDetached(UblockSharedPtr dev) override;

    int PrepareRebuild(string name) override;
    void RebuildDone(string name) override;

    bool ArrayExists(string name);
    virtual int Load(list<string>& failedArrayList);
    int GetAbrList(vector<ArrayBootRecord>& abrList);
    IArrayInfo* GetArrayInfo(string name);
    virtual int ResetMbr(void);

private:
    ArrayComponents* _FindArray(string name);
    ArrayDevice* _FindDevice(string devSn);
    int _Load(string name);
    void _Erase(string name);
    map<string, ArrayComponents*> arrayList;
    ArrayRebuilder* arrayRebuilder = nullptr;
    AbrManager* abrManager = nullptr;
};
using ArrayMgr = Singleton<ArrayManager>;
} // namespace pos
