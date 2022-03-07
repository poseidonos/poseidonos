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

#include "src/metafs/mvm/volume/meta_volume.h"

namespace pos
{
class MockMetaVolume : public MetaVolume
{
public:
    using MetaVolume::MetaVolume;
    MOCK_METHOD(void, InitVolumeBaseLpn, (), (override));
    MOCK_METHOD(bool, IsOkayToStore,
        (FileSizeType fileByteSize, MetaFilePropertySet& prop), (override));
    MOCK_METHOD(void, Init, (MetaStorageSubsystem* metaStorage));
    MOCK_METHOD(bool, CreateVolume, ());
    MOCK_METHOD(MetaVolumeType, GetVolumeType, ());
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, ());
    MOCK_METHOD(MetaLpnType, GetBaseLpn, ());
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (MetaRegionType regionType));
    MOCK_METHOD(bool, OpenVolume, (MetaLpnType* info, bool isNPOR));
    MOCK_METHOD(bool, CloseVolume, (MetaLpnType* info, bool& resetContext));
    MOCK_METHOD(bool, TrimData, (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(FileControlResult, CreateFile,
        (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(FileControlResult, DeleteFile,
        (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(size_t, GetAvailableSpace, ());
    MOCK_METHOD(bool, CheckFileInActive, (FileDescriptorType fd));
    MOCK_METHOD(POS_EVENT_ID, AddFileInActiveList, (FileDescriptorType fd));
    MOCK_METHOD(void, RemoveFileFromActiveList, (FileDescriptorType fd));
    MOCK_METHOD(bool, IsGivenFileCreated, (StringHashType fileKey));
    MOCK_METHOD(FileSizeType, GetFileSize,
        (FileDescriptorType fd));
    MOCK_METHOD(FileSizeType, GetDataChunkSize,
        (FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetFileBaseLpn,
        (FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetMaxLpn,
        ());
    MOCK_METHOD(void, GetInodeList,
        (std::vector<MetaFileInfoDumpCxt>*& fileInfoList));
    MOCK_METHOD(MetaFileInode&, GetInode,
        (FileDescriptorType fd));
    MOCK_METHOD(bool, CopyInodeToInodeInfo,
        (FileDescriptorType fd, MetaFileInodeInfo* inodeInfo /* output */));
    MOCK_METHOD(std::string, LookupNameByDescriptor,
        (FileDescriptorType fd));
    MOCK_METHOD(FileDescriptorType, LookupDescriptorByName,
        (std::string& fileName));
    MOCK_METHOD(POS_EVENT_ID, DetermineVolumeToCreateFile,
        (FileSizeType fileByteSize,
            MetaFilePropertySet& prop, MetaVolumeType volumeType));
};

} // namespace pos
