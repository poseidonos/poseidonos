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

#ifndef VOLUME_H_
#define VOLUME_H_

#include <string>

#include "src/volume/volume_base.h"

namespace pos
{
class Volume : public VolumeBase
{
public:
    Volume(std::string arrayName, int arrayIdx, std::string volName, uint64_t volSizeByte,
        VolumeAttribute volumeAttribute = VolumeAttribute::UserData)
    : VolumeBase(arrayName, arrayIdx, volName, volSizeByte, volumeAttribute)
    {
    }
    Volume(std::string arrayName, int arrayIdx, std::string volName, std::string uuid, uint64_t volSizeByte,
        uint64_t maxiops, uint64_t miniops, uint64_t maxbw, uint64_t minbw, VolumeAttribute volumeAttribute = VolumeAttribute::UserData)
    : VolumeBase(arrayName, arrayIdx, volName, uuid, volSizeByte, maxiops, miniops, maxbw, minbw, volumeAttribute)
    {
    }
    ~Volume(void) override
    {
    }
};

} // namespace pos

#endif // VOLUME_H_
