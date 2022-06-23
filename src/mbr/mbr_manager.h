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

#ifndef MBR_MANAGER_H_
#define MBR_MANAGER_H_

#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "e2e_protect.h"
#include "src/array/interface/i_abr_control.h"
#include "src/array/meta/array_meta.h"
#include "src/bio/ubio.h"
#include "src/device/device_manager.h"
#include "src/include/meta_const.h"
#include "src/mbr/mbr_info.h"
#include "src/mbr/mbr_map_manager.h"
#include "src/mbr/mbr_util.h"

using namespace std;

namespace pos
{
using DeviceIterFunc = function<void(UblockSharedPtr, void*)>;

class MbrManager
{
public:
    MbrManager(void);
    MbrManager(DataProtect* dp, string uuid, DeviceIterFunc iterForDiskIo,
        DeviceIterFunc iterForRead, DeviceManager* devMgr, MbrMapManager* mapMgr);
    virtual ~MbrManager(void);

    int CreateAbr(ArrayMeta& meta);
    int DeleteAbr(string arrayName);
    virtual void GetAbr(string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex);
    virtual int SaveMbr(void);
    int LoadMbr(void);
    int ResetMbr(void);
    void InitDisk(UblockSharedPtr dev);
    int GetAbrList(std::vector<ArrayBootRecord>& abrList);
    virtual struct masterBootRecord& GetMbr(void);
    virtual int GetMbrVersionInMemory(void);
    virtual int UpdateDeviceIndexMap(string arrayName);
    virtual string FindArrayWithDeviceSN(string devSN);

private:
    struct DiskIoContext
    {
        UbioDir ubioDir;
        void* mem;
    };

    struct masterBootRecord systeminfo;
    void _SetParity(void* mem);
    int _VerifyParity(void* mem);
    void _DiskIo(UblockSharedPtr dev, void* ctx);
    void _IterateReadFromDevices(UblockSharedPtr dev, void* ctx);
    int _WriteToDevices(void);
    int _ReadFromDevices(void);
    int _VerifySystemUuid(void* mem);
    int _GetVersion(void* mem);
    int _GetLatestDataList(list<void*> mems, list<void*>* latestMems);
    string _GetSystemUuid(void);
    bool _AllocMem(void);
    int _LoadIndexMap(void);
    string Serialize(void);

    const int MBR_CHUNKS = 1;
    const int MBR_BLOCKS = MBR_CHUNKS * BLOCKS_IN_CHUNK;
    const uint64_t MBR_SIZE = CHUNK_SIZE;
    const uint32_t MBR_ADDRESS = 0;
    const uint32_t MBR_PARITY_SIZE = 4;

    vector<UblockSharedPtr> devs;

    string systemUuid = "";
    int version = 0;
    DataProtect* dataProtect;
    void* mbrBuffer = nullptr;
    DeviceIterFunc diskIoFunc;
    DeviceIterFunc iterateReadFunc;

    pthread_rwlock_t mbrLock;
    map<string, unsigned int> arrayIndexMap;
    using arrayIndexMapIter = map<string, unsigned int>::iterator;

    DeviceManager* devMgr;
    MbrMapManager* mapMgr;
};

} // namespace pos

#endif // MBR_MANAGER_H_
