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
#include <memory>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_container.h"

namespace pos
{
class MockMetaVolumeContainer : public MetaVolumeContainer
{
public:
    using MetaVolumeContainer::MetaVolumeContainer;

    MOCK_METHOD(void, InitContext, (const MetaVolumeType volumeType, const int arrayId,
                const MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage,
                std::shared_ptr<MetaVolume> vol));

    MOCK_METHOD(bool, CreateVolume, (const MetaVolumeType volumeType));
    MOCK_METHOD(bool, OpenAllVolumes, (bool isNPOR));
    MOCK_METHOD(bool, CloseAllVolumes, (bool& resetContext /*output*/));
    MOCK_METHOD(bool, IsGivenVolumeExist, (const MetaVolumeType volumeType));

    MOCK_METHOD(bool, TrimData, (const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg));

    MOCK_METHOD(bool, CreateFile, (const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(bool, DeleteFile, (const MetaVolumeType volumeType, MetaFsFileControlRequest& reqMsg));

    MOCK_METHOD(size_t, GetAvailableSpace, (const MetaVolumeType volumeType));
    MOCK_METHOD(bool, CheckFileInActive, (const MetaVolumeType volumeType, const FileDescriptorType fd));
    MOCK_METHOD(POS_EVENT_ID, AddFileInActiveList, (const MetaVolumeType volumeType, const FileDescriptorType fd));
    MOCK_METHOD(bool, IsGivenFileCreated, (std::string& fileName));
    MOCK_METHOD(void, RemoveFileFromActiveList, (const MetaVolumeType volumeType, const FileDescriptorType fd));
    MOCK_METHOD(FileSizeType, GetFileSize, (const MetaVolumeType volumeType, const FileDescriptorType fd));
    MOCK_METHOD(FileSizeType, GetDataChunkSize, (const MetaVolumeType volumeType, const FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetFileBaseLpn, (const MetaVolumeType volumeType, const FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetMaxLpn, (const MetaVolumeType volumeType));
    MOCK_METHOD(FileDescriptorType, LookupFileDescByName, (std::string& fileName));
    MOCK_METHOD(MetaFileInode&, GetInode, (FileDescriptorType fd, const MetaVolumeType volumeType));
    MOCK_METHOD(void, GetInodeList, (std::vector<MetaFileInfoDumpCxt>*& fileInfoList, const MetaVolumeType volumeType));
    MOCK_METHOD(bool, CopyInodeToInodeInfo,
                    (FileDescriptorType fd, const MetaVolumeType volumeType,
                    MetaFileInodeInfo* inodeInfo /* output */));

    MOCK_METHOD(POS_EVENT_ID, DetermineVolumeToCreateFile,
                    (FileSizeType fileByteSize, MetaFilePropertySet& prop,
                        const MetaVolumeType volumeType));
    MOCK_METHOD(POS_EVENT_ID, LookupMetaVolumeType,
                    (FileDescriptorType fd, const MetaVolumeType volumeType));
    MOCK_METHOD(POS_EVENT_ID, LookupMetaVolumeType,
                    (std::string& fileName, const MetaVolumeType volumeType));
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, (const MetaVolumeType volumeType));
};

} // namespace pos
