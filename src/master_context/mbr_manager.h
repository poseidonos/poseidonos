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

#ifndef MBR_MANAGER_H_
#define MBR_MANAGER_H_

#include <functional>
#include <list>
#include <string>
#include <vector>

#include "e2e_protect.h"
#include "src/array/meta/array_meta.h"
#include "src/include/meta_const.h"
#include "src/io/general_io/ubio.h"
#include "src/lib/singleton.h"
#include "src/master_context/mbr_info.h"

using namespace std;

namespace ibofos
{
using DeviceIterFunc = function<void(UBlockDevice*, void*)>;

class MbrManager
{
public:
    MbrManager(void);
    ~MbrManager(void);
    int Write(int arrayNum, ArrayMeta& meta);
    int SetMfsInit(int arrayNum, bool value);
    bool GetMfsInit(int arrayNum);
    int Reset(void);
    int LoadMeta(int arrayNum, ArrayMeta& meta, string targetArray);
    int RebuildMbr(void);
    int Read(void);

private:
    struct DiskIoContext
    {
        UbioDir ubioDir;
        void* mem;
    };

    struct systemBootRecord systeminfo;

    void _SetParity(void* mem);
    int _VerifyParity(void* mem);
    void _DiskIo(UBlockDevice* dev, void* ctx);
    void _IterateReadFromDevices(UBlockDevice* dev, void* ctx);
    int _WriteToDevices(void);
    int _ReadFromDevices(void);
    int _VerifySystemUuid(void* mem);
    int _GetVersion(void* mem);
    int _GetLatestDataList(list<void*> mems, list<void*>* latestMems);
    string _GetSystemUuid(void);
    bool _IsValidDevice(UBlockDevice* ublockDev);
    bool _AllocMem(void);
    string _ParseMbrData(void);
    int _CopyData(char* dest, string src, size_t len);

    const int MBR_CHUNKS = 1;
    const int MBR_BLOCKS = MBR_CHUNKS * BLOCKS_IN_CHUNK;
    const uint64_t MBR_SIZE = CHUNK_SIZE;
    const uint32_t MBR_ADDRESS = 0;
    const uint32_t MBR_PARITY_SIZE = 4;

    vector<UBlockDevice*> devs;

    string systemUuid = "";
    int version = 0;

    DataProtect* dataProtect;
    void* mbrBuffer = nullptr;
    DeviceIterFunc diskIoFunc;
    DeviceIterFunc iterateReadFunc;
};

using MbrManagerSingleton = Singleton<MbrManager>;

} // namespace ibofos

#endif // MBR_MANAGER_H_
