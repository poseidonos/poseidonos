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

#pragma once

#include <string>
#include <vector>

#include "meta_storage_specific.h"
#include "mf_lock_type.h"
#include "mfs_common.h"
#include "mfs_file_property.h"
#include "mfs_wbt_ret_dataformat.h"

enum class MetaFsMoMReqType
{
    OnVolumeSpcfReq_Base = 0,
    // File Management Basic
    FileOpen = OnVolumeSpcfReq_Base,
    FileClose,
    FileCreate,
    // FileFormat,

    // File Management (Advanced)
    // FileReconfig,
    FileDelete,
    // FileResize,
    // FileCopy,

    GetDataChunkSize,
    CheckFileExist,
    CheckFileAccessible,
    GetFileSize,
    GetTargetMediaType,
    GetFileBaseLpn,

    GetTheBiggestExtentSize,

    OnVolumeSpecificReq_Max,
    // -------------------------
    NonVolumeSpcfReq_Base = OnVolumeSpecificReq_Max,

    GetMetaFileInfoList = NonVolumeSpcfReq_Base,
    GetMaxFileSizeLimit,
    GetFileInode,
    EstimateDataChunkSize,

    NonVolumeSpecificReq_Max,

    Max = NonVolumeSpecificReq_Max,

    OnVolumeSpcfReq_Count = OnVolumeSpecificReq_Max - OnVolumeSpcfReq_Base,
    NonVolumeSpcfReq_Count = NonVolumeSpecificReq_Max - NonVolumeSpcfReq_Base,
};

union MetaFsMoMReqComplData {
public:
    MetaFsMoMReqComplData(void)
    {
        memset(all, 0, sizeof(MetaFsMoMReqComplData));
    }

    static const uint32_t META_OF_META_REQ_COMPLETION_DATA_SIZE = 64;

    union {
        FileFDType openfd;
        FileSizeType dataChunkSize;
        FileSizeType fileSize;
        bool fileAccessible;
        MetaStorageType targetMediaType;
        MetaLpnType fileBaseLpn;
        const char* filename;
        std::vector<MetaFileInfoDumpCxt>* fileInfoListPointer;
        FileSizeType maxFileSizeByteLimit;
        MetaFileInodeInfo* inodeInfoPointer;

        uint8_t all[META_OF_META_REQ_COMPLETION_DATA_SIZE];
    };
};

// Meta of Meta Req. Msg
class MetaFsMoMReqMsg
{
public:
    MetaFsMoMReqMsg(void)
    : reqType(MetaFsMoMReqType::Max),
      fd(MetaFsCommonConst::INVALID_FD),
      fileName(nullptr),
      fileByteSize(MetaFsCommonConst::INVALID_BYTE_SIZE),
      lock(MDFileLockTypeEnum::Default)
    {
    }

    bool
    IsValid(void)
    {
        if (reqType >= MetaFsMoMReqType::Max)
        {
            return false;
        }
        return true;
    }

    MetaFsMoMReqType reqType;
    FileFDType fd;
    std::string* fileName;
    FileSizeType fileByteSize;
    MDFileLockTypeEnum lock;

    MetaFilePropertySet fileProperty; // used to file creation & format/reconfig

    MetaFsMoMReqComplData completionData;
};
