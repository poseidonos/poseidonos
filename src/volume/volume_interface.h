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

#include <string>
#include <cstdint>

#include "src/sys_event/volume_event.h"

using namespace std;

namespace pos
{

class VolumeList;
class VolumeBase;
class VolumeEventPublisher;

class VolumeInterface
{
public:
    explicit VolumeInterface(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher = nullptr);
    virtual ~VolumeInterface(void);

protected:
    void _CheckVolumeSize(uint64_t volSize);
    void _SetVolumeQos(VolumeBase* volume, uint64_t maxIops, uint64_t maxBw, uint64_t minIops, uint64_t minBw);
    void _PrintLogVolumeQos(VolumeBase* volume, uint64_t originalMaxIops, uint64_t originalMaxBw, uint64_t originalMinIops, uint64_t originalMinBw);
    int _SaveVolumes(void);

    void _SetVolumeEventBase(VolumeBase* volume, std::string subnqn = "");
    void _SetVolumeEventPerf(VolumeBase* volume);
    void _SetVolumeArrayInfo(void);

    VolumeList& volumeList;
    std::string arrayName;
    int arrayID;
    VolumeEventPublisher* eventPublisher;

    VolumeEventBase volumeEventBase;
    VolumeEventPerf volumeEventPerf;
    VolumeArrayInfo volumeArrayInfo;
};

} // namespace pos
