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

namespace pos
{
class MetaVolumeContext
{
public:
    MetaVolumeContext(void);
    ~MetaVolumeContext(void);

    void InitContext(MetaVolumeType volumeType, int arrayId,
            MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage);

    bool CreateVolume(MetaVolumeType volumeType);
    bool Open(bool isNPOR);
    bool Close(bool& resetCxt /*output*/);

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(bool isNPOR);
#endif

    bool IsFileInodeExist(std::string& fileName);
    MetaLpnType GetGlobalMaxFileSizeLimit(void);
    FileSizeType CalculateDataChunkSizeInPage(MetaFilePropertySet& prop);

    // volume container
    std::pair<MetaVolumeType, POS_EVENT_ID> DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(FileDescriptorType fd);
    std::pair<MetaVolumeType, POS_EVENT_ID> LookupMetaVolumeType(std::string& fileName);
    MetaVolume& GetMetaVolume(MetaVolumeType volType);
    bool IsGivenVolumeExist(MetaVolumeType volType);
    bool GetVolOpenFlag(void);
    MetaLpnType GetMaxMetaLpn(MetaVolumeType volType);

    // file mgr
    size_t GetTheBiggestExtentSize(MetaVolumeType volType);
    bool CheckFileInActive(MetaVolumeType volType, FileDescriptorType fd);
    POS_EVENT_ID AddFileInActiveList(MetaVolumeType volType, FileDescriptorType fd);
    void RemoveFileFromActiveList(MetaVolumeType volType, FileDescriptorType fd);
    bool TrimData(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);

    // inode mgr
    bool CreateFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    bool DeleteFileInode(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg);
    FileSizeType GetFileSize(MetaVolumeType volType, FileDescriptorType fd);
    FileSizeType GetDataChunkSize(MetaVolumeType volType, FileDescriptorType fd);
    MetaLpnType GetFileBaseLpn(MetaVolumeType volType, FileDescriptorType fd);

    // fd mgr
    FileDescriptorType LookupFileDescByName(std::string& fileName);

private:
    MetaVolume* _InitVolume(MetaVolumeType volType, int arrayId,
                    MetaLpnType maxLpnNum, MetaStorageSubsystem* metaStorage);
    void _SetGlobalMaxFileSizeLimit(MetaLpnType maxVolumeLpn);

    FileDescriptorType _AllocFileDescriptor(std::string& fileName);
    void _FreeFileDescriptor(std::string& fileName, FileDescriptorType fd);
    MetaFilePageMap _AllocExtent(MetaVolumeType volType, FileSizeType fileSize);
    void _UpdateVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd, MetaVolumeType volumeType);
    void _RemoveVolumeLookupInfo(StringHashType fileHashKey, FileDescriptorType fd);
    void _RemoveExtent(MetaVolumeType volType, MetaLpnType baseLpn, FileSizeType fileSize);
    void _InsertFileDescLookupHash(std::string& fileName, FileDescriptorType fd);
    void _RemoveFileDescLookupHash(std::string& fileName);
    bool _CopyExtentContent(MetaVolumeType volType);

    void _BuildFreeFDMap(void);
    void _BuildFileNameLookupTable(void);

    MetaFileManager& _GetFileManager(MetaVolumeType volType);
    MetaFileInodeManager& _GetInodeManager(MetaVolumeType volType);

    MetaVolumeContainer volumeContainer;
    FileDescriptorManager fdMgr;
    MetaLpnType maxFileSizeLpnLimit;
};
} // namespace pos
