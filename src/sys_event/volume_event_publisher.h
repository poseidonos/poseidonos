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


#include "src/include/array_mgmt_policy.h"
#include "src/lib/singleton.h"
#include "src/sys_event/volume_event.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace pos
{
class VolumeEventPublisher
{
public:
    VolumeEventPublisher(void);
    virtual ~VolumeEventPublisher(void);
    void RegisterSubscriber(VolumeEvent* subscriber, std::string arrayName, int arrayID);
    void RemoveSubscriber(VolumeEvent* subscriber, std::string arrayName, int arrayID);
    bool NotifyVolumeCreated(std::string volName, int volID,
        uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID);
    bool NotifyVolumeUpdated(std::string volName, int volID,
        uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID);
    bool NotifyVolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName, int arrayID);
    bool NotifyVolumeMounted(std::string volName, std::string subnqn, int volID,
        uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID);
    bool NotifyVolumeUnmounted(std::string volName, int volID, std::string arrayName, int arrayID);
    bool NotifyVolumeLoaded(std::string name, int id,
        uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID);
    void NotifyVolumeDetached(vector<int> volList, std::string arrayName, int arrayID);

    int RemoveArrayIdx(std::string arrayName);
    int GetArrayIdx(std::string arrayName);

private:
    int _SetArrayIdx(std::string arrayName);

    vector<std::pair<int, VolumeEvent*>> subscribers;
    std::string arrayNameList[ArrayMgmtPolicy::MAX_ARRAY_CNT];
    VolumeEvent* qosManagerVolumeEvent;
};

using VolumeEventPublisherSingleton = Singleton<VolumeEventPublisher>;
} // namespace pos
