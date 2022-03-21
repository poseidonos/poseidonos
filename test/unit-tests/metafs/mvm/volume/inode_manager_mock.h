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
#include <unordered_map>
#include <vector>

#include "src/metafs/mvm/volume/inode_manager.h"

namespace pos
{
class MockInodeManager : public InodeManager
{
public:
    using InodeManager::InodeManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(void, SetMetaFileBaseLpn, (const MetaLpnType lpn), (const));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem * mss), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
    MOCK_METHOD(MetaLpnType, GetRegionBaseLpn, (const MetaRegionType regionType), (const));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (const MetaRegionType regionType), (const));
    MOCK_METHOD(MetaLpnType, GetMetaFileBaseLpn, (), (const));
    MOCK_METHOD(void, CreateInitialInodeContent, (const uint32_t maxInodeNum), (const));
    MOCK_METHOD(bool, LoadContent, ());
    MOCK_METHOD(bool, BackupContent,
        (MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts,
            MetaLpnType iNodeTableLpnCnts));
    MOCK_METHOD(bool, RestoreContent,
        (MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts,
            MetaLpnType iNodeTableLpnCnts));
    MOCK_METHOD(POS_EVENT_ID, AddFileInActiveList, (const FileDescriptorType fd));
    MOCK_METHOD(bool, CheckFileInActive, (const FileDescriptorType fd), (const));
    MOCK_METHOD(void, RemoveFileFromActiveList, (const FileDescriptorType fd));
    MOCK_METHOD(size_t, GetFileCountInActive, (), (const));
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, (), (const));
    MOCK_METHOD(void, PopulateFDMapWithVolumeType, (FileDescriptorInVolume & dest), (const));
    MOCK_METHOD(void, PopulateFileNameWithVolumeType, (FileHashInVolume & dest), (const));
    MOCK_METHOD(uint32_t, GetExtent,
        (const FileDescriptorType fd, std::vector<MetaFileExtent>& extents), (const));
    MOCK_METHOD(MetaLpnType, GetAvailableLpnCount, (), (const));
    MOCK_METHOD(size_t, GetAvailableSpace, (), (const));
    MOCK_METHOD(MetaFileInode&, GetInodeEntry, (const uint32_t entryIdx), (const));
    MOCK_METHOD(bool, IsFileInodeInUse, (const FileDescriptorType fd), (const));
    MOCK_METHOD(bool, IsGivenFileCreated, (const StringHashType fileKey), (const));
    MOCK_METHOD(FileSizeType, GetFileSize, (const FileDescriptorType fd), (const));
    MOCK_METHOD(FileSizeType, GetDataChunkSize, (const FileDescriptorType fd), (const));
    MOCK_METHOD(MetaLpnType, GetFileBaseLpn, (const FileDescriptorType fd), (const));
    MOCK_METHOD(MetaFileInode&, GetFileInode, (const FileDescriptorType fd), (const));
    MOCK_METHOD(FileDescriptorType, LookupDescriptorByName, (const std::string& fileName), (const));
    MOCK_METHOD(std::string, LookupNameByDescriptor, (const FileDescriptorType fd), (const));
};

} // namespace pos
