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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta Volume Manager
*/
#pragma once

#include <string>
#include <utility>

#include "file_descriptor_manager.h"
#include "meta_file_manager.h"
#include "meta_volume.h"
#include "volume_catalog_manager.h"
#include "meta_volume_container.h"
#include "mf_inode_mgr.h"
#include "metafs_common.h"
#include "metafs_control_request.h"
#include "nvram_meta_volume.h"
#include "ssd_meta_volume.h"
#include "meta_volume_container_map.h"

namespace pos
{
class MetaVolumeContext
{
public:
    MetaVolumeContext(void);
    ~MetaVolumeContext(void);

    void InitContext(MetaVolumeType volumeType, std::string arrayName, MetaLpnType maxVolPageNum);

    bool CreateVolume(MetaVolumeType volumeType, std::string arrayName);
    bool Open(bool isNPOR, std::string arrayName);
    bool Close(bool& resetCxt /*output*/, std::string arrayName);

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(bool isNPOR, std::string arrayName);
#endif

    bool IsFileInodeExist(std::string& fileName, std::string& arrayName);
    MetaLpnType GetGlobalMaxFileSizeLimit(void);
    FileSizeType CalculateDataChunkSizeInPage(MetaFilePropertySet& prop);

    // volume container
    std::pair<MetaVolumeType, POS_EVENT_ID> DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop, std::string arrayName);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(FileDescriptorType fd, std::string arrayName);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(std::string& fileName, std::string arrayName);
    MetaVolume& GetMetaVolume(MetaVolumeType volType, std::string arrayName);
    bool IsGivenVolumeExist(MetaVolumeType volType, std::string arrayName);
    bool GetVolOpenFlag(std::string arrayName);
    MetaLpnType GetMaxMetaLpn(MetaVolumeType volType, std::string arrayName);

    // file mgr
    size_t GetTheBiggestExtentSize(MetaVolumeType volType, std::string arrayName);
    bool CheckFileInActive(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName);
    POS_EVENT_ID AddFileInActiveList(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName);
    void RemoveFileFromActiveList(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName);
    bool TrimData(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    // inode mgr
    bool CreateFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    bool DeleteFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    FileSizeType GetFileSize(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName);
    FileSizeType GetDataChunkSize(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName);
    MetaLpnType GetFileBaseLpn(MetaVolumeType volType, FileDescriptorType fd, std::string arrayName);

    // fd mgr
    FileDescriptorType LookupFileDescByName(std::string& fileName, std::string arrayName);

private:
    MetaVolume* _InitVolume(MetaVolumeType volType, std::string arrayName, MetaLpnType maxLpnNum);
    void _SetGlobalMaxFileSizeLimit(MetaLpnType maxVolumeLpn);

    FileDescriptorType _AllocFileDescriptor(std::string& fileName, std::string& arrayName);
    void _FreeFileDescriptor(std::string& fileName, FileDescriptorType fd, std::string& arrayName);
    MetaFilePageMap _AllocExtent(MetaVolumeType volType, FileSizeType fileSize, std::string& arrayName);
    void _UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd,
                        std::string arrayName, MetaVolumeType volumeType);
    void _RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd,
                        std::string arrayName);
    void _RemoveExtent(MetaVolumeType volType, MetaLpnType baseLpn, FileSizeType fileSize, std::string& arrayName);
    void _InsertFileDescLookupHash(std::string& fileName, FileDescriptorType fd, std::string& arrayName);
    void _RemoveFileDescLookupHash(std::string& fileName, std::string& arrayName);
    bool _CopyExtentContent(MetaVolumeType volType, std::string arrayName);

    void _BuildFreeFDMap(std::string arrayName);
    void _BuildFileNameLookupTable(std::string arrayName);

    MetaFileManager& _GetFileManager(MetaVolumeType volType, std::string arrayName);
    MetaFileInodeManager& _GetInodeManager(MetaVolumeType volType, std::string arrayName);

    MetaVolumeContainerMap volumeMap;
    MetaLpnType maxFileSizeLpnLimit;
};
} // namespace pos
