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

#include <string.h>

#include <string>

#include "meta_storage_specific.h"
#include "meta_volume_type.h"
#include "metafs_type.h"
#include "src/meta_file_intf/meta_file_include.h"

namespace pos
{
class MetaFsMediaToVolume
{
public:
    MetaStorageType media;
    MetaVolumeType volumeType;
};

class MetaFsVolumeToMedia
{
public:
    MetaVolumeType volumeType;
    MetaStorageType media;
};

class MetaFileUtil
{
public:
    static StringHashType GetHashKeyFromFileName(const std::string& fileName);
    static MetaStorageType ConvertToMediaType(MetaVolumeType volume);
    static std::string ConvertToMediaTypeName(MetaVolumeType volume);
    static MetaVolumeType ConvertToVolumeType(MetaStorageType media);
    static uint64_t GetEpochSignature(std::time_t t = std::time(0));

private:
    static const MetaFsMediaToVolume MEDIA_TO_VOLUME[(uint32_t)MetaStorageType::Max];
    static const MetaFsVolumeToMedia VOLUME_TO_MEDIA[(uint32_t)MetaVolumeType::Max];
};
} // namespace pos
