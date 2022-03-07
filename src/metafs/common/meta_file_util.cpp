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

#include "meta_file_util.h"

#include <string.h>

namespace pos
{
static_assert(((int)MetaStorageType::SSD) == 0, "error");
const MetaFsMediaToVolume MetaFileUtil::MEDIA_TO_VOLUME[] =
    {
        {MetaStorageType::SSD, MetaVolumeType::SsdVolume},
        {MetaStorageType::NVRAM, MetaVolumeType::NvRamVolume},
        {MetaStorageType::JOURNAL_SSD, MetaVolumeType::JournalVolume}};
static_assert(((int)MetaVolumeType::SsdVolume) == 0, "error");
const MetaFsVolumeToMedia MetaFileUtil::VOLUME_TO_MEDIA[] =
    {
        {MetaVolumeType::SsdVolume, MetaStorageType::SSD},
        {MetaVolumeType::NvRamVolume, MetaStorageType::NVRAM},
        {MetaVolumeType::JournalVolume, MetaStorageType::JOURNAL_SSD}};

StringHashType
MetaFileUtil::GetHashKeyFromFileName(const std::string& fileName)
{
    return std::hash<std::string>{}(fileName.c_str());
}

MetaStorageType
MetaFileUtil::ConvertToMediaType(MetaVolumeType volume)
{
    return VOLUME_TO_MEDIA[(uint32_t)volume].media;
}

std::string
MetaFileUtil::ConvertToMediaTypeName(MetaVolumeType volume)
{
    switch (volume)
    {
        case MetaVolumeType::SsdVolume:
        {
            return std::string("SSD array");
        }
        break;
        case MetaVolumeType::NvRamVolume:
        {
            return std::string("NVRAM/NVDIMM");
        }
        break;
        default:
        {
            assert(false);
        }
    }
}

MetaVolumeType
MetaFileUtil::ConvertToVolumeType(MetaStorageType media)
{
    return MEDIA_TO_VOLUME[(uint32_t)media].volumeType;
}

uint64_t
MetaFileUtil::GetEpochSignature(std::time_t t)
{
    // CreateMBR, Need to add more context
    // std::time_t t = std::time(0); // get time now -> get from param
    std::tm* now = std::localtime(&t);

    // 0000_YYYY_MMDD_HHMM_
    uint64_t sign = (uint64_t)(now->tm_year + 1900) * 10000000000 +
        (uint64_t)(now->tm_mon + 1) * 100000000 +
        (uint64_t)now->tm_mday * 1000000 +
        (uint64_t)now->tm_hour * 10000 +
        (uint64_t)now->tm_min * 100 +
        (uint64_t)now->tm_sec;

    return sign;
}
} // namespace pos
