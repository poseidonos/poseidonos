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

#ifndef __VOLUME_EVENT__
#define __VOLUME_EVENT__

#include <string>
#include <vector>

using namespace std;
namespace pos
{

struct VolumeEventBase
{
    int volId;
    uint64_t volSizeByte;
    string volName;
    string uuid;
    string subnqn;
    bool isPrimaryRole;
};

struct VolumeEventPerf
{
    uint64_t maxiops;
    uint64_t maxbw;
};

struct VolumeArrayInfo
{
    int arrayId;
    string arrayName;
};

class VolumeEvent
{
public:
    VolumeEvent(string _tag, string _arrayName, int _arrayId);
    virtual ~VolumeEvent(void);
    string Tag(void);
    void SetVolumeBase(VolumeEventBase* volEventBase, int volId, uint64_t volSizeByte, string volName, string uuid,
        string subnqn, bool _isPrimaryRole);
    void SetVolumePerf(VolumeEventPerf* volEventPerf, uint64_t maxiops, uint64_t maxbw);
    void SetVolumeArrayInfo(VolumeArrayInfo* volArrayInfo, int arrayId, string arrayName);

    virtual int VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) = 0;
    virtual int VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) = 0;
    virtual int VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) = 0;
    virtual int VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) = 0;
    virtual int VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) = 0;
    virtual int VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) = 0;
    virtual int VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) = 0;

protected:
    string arrayName = "";
    int arrayId;

private:
    string tag = "";
};
} // namespace pos

#endif // __VOLUME_EVENT__
