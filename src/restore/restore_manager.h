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

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "src/cli/command_processor.h"
#include "src/lib/singleton.h"

using namespace std;
using namespace rapidjson;

const static string restore_file = "/etc/pos/restore.json";

namespace pos
{
class RestoreManager
{
public:
    RestoreManager();
    virtual ~RestoreManager();

    bool TransportCreate(string trType, uint32_t bufCacheSize, uint32_t numSharedBuf, uint32_t ioUnitSize);
    bool SubsystemCreate(string subnqn, string serialNumber, string modelNumber, uint32_t maxNamespaces, bool allowAnyHost, bool anaReporting);
    bool SubsystemDelete(string subnqn);
    bool ListenerAdd(string subnqn, string trType, string trAddr, string trSvcid);
    bool ListenerRemove(string subnqn, string trType, string trAddr, string trSvcid);
    bool DeviceCreate(string name, string type, uint32_t blockSize, uint32_t numBlocks, uint32_t numa);
    bool ArrayCreate(string name);
    bool ArrayDelete(string name);
    bool SetArrayMountState(string name, bool isMount, bool isWt, string trAddr = "");
    bool VolumeCreate(string arrayName, string volName, int32_t nsid);
    bool VolumeDelete(string arrayName, string volName);
    bool SetVolumeMountState(string arrayName, string volName, bool isMount, string subNqn = "", int32_t nsid = 0);
    bool VolumeRename(string arrayName, string volOldName, string volNewName);

    void ClearRestoreState();
    bool Restore();
    void Print();
    void EnableStateSave();

private:
    bool _ReadJson(void);
    bool _WriteJson(void);
    bool _ArrayRestore(void);
    bool _VolumeRestore(void);
    bool _TransportRestore(void);
    bool _DeviceRestore(void);
    bool _SubsystemRestore(void);
    bool _ScanDevice(void);
    bool _CheckJsonWrite(void);

    bool saveEnabled;
    Document* jsonDocument;
    CommandProcessor* commandProcessor;
};

using RestoreManagerSingleton = Singleton<RestoreManager>;

} // namespace pos