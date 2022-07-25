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

#include <gmock/gmock.h>
#include <list>
#include <string>
#include <vector>
#include "src/metafs/mai/metafs_file_control_api.h"

namespace pos
{
class MockMetaFsFileControlApi : public MetaFsFileControlApi
{
public:
    using MetaFsFileControlApi::MetaFsFileControlApi;
    MOCK_METHOD(POS_EVENT_ID, Create,
        (std::string & fileName, uint64_t fileByteSize,
            MetaFilePropertySet prop, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Delete,
        (std::string & fileName, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Open,
        (std::string & fileName, int& fd, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Close,
        (FileDescriptorType fd, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, CheckFileExist,
        (std::string & fileName, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(size_t, GetFileSize,
        (int fd, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(size_t, GetAlignedFileIOSize,
        (int fd, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(size_t, EstimateAlignedFileIOSize,
        (MetaFilePropertySet & prop, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(size_t, GetAvailableSpace,
        (MetaFilePropertySet & prop, MetaVolumeType volumeType),
        (override));
    MOCK_METHOD(size_t, GetMaxMetaLpn, (MetaVolumeType type), (override));
    MOCK_METHOD(void, SetStatus, (bool isNormal));
    MOCK_METHOD(MetaFileContext*, GetFileInfo,
        (FileDescriptorType fd, MetaVolumeType type));
    MOCK_METHOD(std::vector<MetaFileInfoDumpCxt>, Wbt_GetMetaFileList,
        (MetaVolumeType type),
        (override));
    MOCK_METHOD(MetaFileInodeInfo*, Wbt_GetMetaFileInode,
        (std::string & fileName, MetaVolumeType type),
        (override));
    MOCK_METHOD(void, InitVolume,
        (MetaVolumeType volType, int arrayId, MetaLpnType maxVolPageNum),
        (override));
    MOCK_METHOD(bool, CreateVolume, (MetaVolumeType volType), (override));
    MOCK_METHOD(bool, OpenVolume, (bool isNPOR), (override));
    MOCK_METHOD(bool, CloseVolume, (bool& isNPOR), (override));
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, (MetaVolumeType type));
};

} // namespace pos
