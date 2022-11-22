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

#ifndef VOLUME_BASE_H_
#define VOLUME_BASE_H_

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

#include "src/volume/volume_attribute.h"
#include "src/volume/volume_network_property.h"
#include "src/volume/volume_perfomance_property.h"
#include "src/volume/volume_replicate_property.h"
#include "src/volume/volume_status_property.h"

#define MAX_VOLUME_COUNT (256)

namespace pos
{

enum VolumeIoType
{
    UserRead,
    UserWrite,
    InternalIo,
    MaxVolumeIoTypeCnt
};

class VolumeBase : public VolumeAttribute, public StatusProperty, public NetworkProperty, public PerfomanceProperty, public ReplicationProperty
{
public:
    VolumeBase(int arrayIdx, std::string arrayName, DataAttribute dataAttribute,
                std::string volName, uint64_t volSizeByte, uint32_t nsid,
                ReplicationRole voluemRole);
    VolumeBase(int arrayIdx, std::string arrayName, DataAttribute dataAttribute, std::string inputUuid,
                std::string volName, uint64_t volSizeByte, uint32_t nsid,
                uint64_t _maxiops, uint64_t _miniops, uint64_t _maxbw, uint64_t _minbw,
                ReplicationRole voluemRole);
    virtual ~VolumeBase(void);

    int Mount(void);
    int Unmount(void);

    void LockStatus(void);
    void UnlockStatus(void);

    uint64_t UsedSize(void);
    uint64_t RemainingSize(void);

    bool IsValid(void) {return isValid;}
    void SetValid(bool valid) {isValid = valid;}

    int ID;

protected:  
    bool isValid = true;
    std::mutex statusMutex;
    static const int INVALID_VOL_ID = -1;
};

} // namespace pos

#endif // VOLUME_BASE_H_
