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

#include <string>
#include <vector>

#include "src/metafs/include/meta_storage_specific.h"
#include "src/metafs/include/meta_volume_type.h"
#include "src/metafs/include/mf_lock_type.h"
#include "src/metafs/common/metafs_common.h"
#include "src/metafs/include/mf_property.h"
#include "src/metafs/include/mf_dataformat.h"

namespace pos
{
enum class MetaFsFileControlType
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
    GetMaxMetaLpn,

    GetAvailableSpace,

    ArrayCreate,
    ArrayDelete,

    OnVolumeSpecificReq_Max,
    // -------------------------
    NonVolumeSpcfReq_Base = OnVolumeSpecificReq_Max,

    GetMetaFileInfoList = NonVolumeSpcfReq_Base,
    GetFileInode,
    EstimateDataChunkSize,

    NonVolumeSpecificReq_Max,

    Max = NonVolumeSpecificReq_Max,

    OnVolumeSpcfReq_Count = OnVolumeSpecificReq_Max - OnVolumeSpcfReq_Base,
    NonVolumeSpcfReq_Count = NonVolumeSpecificReq_Max - NonVolumeSpcfReq_Base,
};

union MetaFsFileControlCompletion {
public:
    MetaFsFileControlCompletion(void)
    {
        memset(all, 0, sizeof(MetaFsFileControlCompletion));
    }

    static const uint32_t META_OF_META_REQ_COMPLETION_DATA_SIZE = 64;

    union {
        FileDescriptorType openfd;
        FileSizeType dataChunkSize;
        FileSizeType fileSize;
        FileSizeType maxLpn;
        bool fileAccessible;
        MetaStorageType targetMediaType;
        MetaLpnType fileBaseLpn;
        const char* filename;
        const char* arrayName;
        std::vector<MetaFileInfoDumpCxt>* fileInfoListPointer;
        FileSizeType maxFileSizeByteLimit;
        MetaFileInodeInfo* inodeInfoPointer;

        uint8_t all[META_OF_META_REQ_COMPLETION_DATA_SIZE];
    };
};

// Meta of Meta Req. Msg
class MetaFsFileControlRequest : public MetaFsRequestBase
{
public:
    MetaFsFileControlRequest(void)
    : reqType(MetaFsFileControlType::Max),
      fd(MetaFsCommonConst::INVALID_FD),
      fileName(nullptr),
      arrayId(0),
      fileByteSize(MetaFsCommonConst::INVALID_BYTE_SIZE),
      volType(MetaVolumeType::Max),
      lock(MetaFileLockType::Default),
      fileProperty()
    {
    }

// LCOV_EXCL_START
    virtual ~MetaFsFileControlRequest(void)
    {
    }
// LCOV_EXCL_STOP

    virtual bool
    IsValid(void)
    {
        if (reqType >= MetaFsFileControlType::Max)
        {
            return false;
        }
        return true;
    }

    MetaFsFileControlType reqType;
    FileDescriptorType fd;
    std::string* fileName;
    int arrayId;
    FileSizeType fileByteSize;
    MetaVolumeType volType;
    MetaFileLockType lock;

    MetaFilePropertySet fileProperty; // used to file creation & format/reconfig

    MetaFsFileControlCompletion completionData;
};
} // namespace pos
