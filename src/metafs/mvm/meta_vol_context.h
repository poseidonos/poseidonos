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
 * iBoFOS - Meta Filesystem Layer
 * 
 * Meta Volume Manager
*/
#pragma once

#include <string>
#include <utility>

#include "meta_fd_mgr.h"
#include "meta_file_mgr.h"
#include "meta_vol_base.h"
#include "meta_vol_catalog_mgr.h"
#include "meta_vol_container.h"
#include "mf_inode_mgr.h"
#include "mfs_common.h"
#include "mfs_mvm_top.h"
#include "mvm_req.h"
#include "nvram_meta_vol.h"
#include "ssd_meta_vol.h"

class MetaVolContext
{
public:
    MetaVolContext(void);
    ~MetaVolContext(void);

    void InitVolume(MetaVolumeType volumeType, MetaLpnType maxVolPageNum);

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
    std::pair<MetaVolumeType, IBOF_EVENT_ID> DetermineVolumeToCreateFile(FileSizeType fileByteSize, MetaFilePropertySet& prop);
    std::pair<MetaVolumeType, IBOF_EVENT_ID> LookupMetaVolumeType(FileFDType fd);
    std::pair<MetaVolumeType, IBOF_EVENT_ID> LookupMetaVolumeType(std::string& fileName);
    MetaVolumeClass& GetMetaVolume(MetaVolumeType volumeType);
    bool IsGivenVolumeExist(MetaVolumeType volumeType);
    bool GetVolOpenFlag(void);
    MetaLpnType GetMaxMetaLpn(MetaVolumeType mediaType);

    // file mgr
    size_t GetTheBiggestExtentSize(MetaVolumeClass& tgtMetaVol);
    bool CheckFileInActive(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    bool IsLockAlreadyGranted(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    void LockFile(MetaVolumeClass& tgtMetaVol, FileFDType fd, MDFileLockTypeEnum lock);
    void UnlockFile(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    IBOF_EVENT_ID AddFileInActiveList(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    void RemoveFileFromActiveList(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    bool TrimData(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);

    // inode mgr
    bool CreateFileInode(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    bool DeleteFileInode(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg);
    FileSizeType GetFileSize(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    FileSizeType GetDataChunkSize(MetaVolumeClass& tgtMetaVol, FileFDType fd);
    MetaLpnType GetFileBaseLpn(MetaVolumeClass& tgtMetaVol, FileFDType fd);

    // fd mgr
    FileFDType LookupFileDescByName(std::string& fileName);

private:
    MetaVolumeClass* _InitVolume(MetaVolumeType volType, MetaLpnType maxLpnNum);
    void _SetGlobalMaxFileSizeLimit(MetaLpnType maxVolumeLpn);

    FileFDType _AllocNewFD(std::string& fileName);
    void _FreeFD(std::string& fileName, FileFDType fd);
    MetaFilePageMap _AllocExtent(MetaVolumeClass& tgtMetaVol, FileSizeType fileSize);
    void _UpdateVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd, MetaVolumeType volumeType);
    void _RemoveVolumeLookupInfo(StringHashType fileHashKey, FileFDType fd);
    void _RemoveExtent(MetaVolumeClass& tgtMetaVol, MetaLpnType baseLpn, FileSizeType fileSize);
    void _InsertFileDescLookupHash(std::string& fileName, FileFDType fd);
    void _RemoveFileDescLookupHash(std::string& fileName);
    bool _CopyExtentContent(MetaVolumeClass& tgtMetaVol);
    MetaFileInode& _GetFileInode(MetaVolumeClass& tgtMetaVol, FileFDType fd);

    void _BuildFreeFDMap(void);
    void _BuildFileNameLookupTable(void);

    MetaVolContainerClass volumeContainer;
    MetaFDMgrClass fdMgr;
    MetaLpnType maxFileSizeLpnLimit;
};
