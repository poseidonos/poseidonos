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

#include "mk/ibof_config.h"

#include "metafs_wbt_api.h"

namespace pos
{
MetaFsWBTApi::MetaFsWBTApi(void)
: arrayId(INT32_MAX),
  ctrl(nullptr)
{
}

MetaFsWBTApi::MetaFsWBTApi(int arrayId, MetaFsFileControlApi* ctrl)
: MetaFsWBTApi()
{
    this->arrayId = arrayId;
    this->ctrl = ctrl;
}

MetaFsWBTApi::~MetaFsWBTApi(void)
{
}

// WBT: Load file inode lists for each volume type
bool
MetaFsWBTApi::GetMetaFileList(std::vector<MetaFileInfoDumpCxt>& result,
                                MetaVolumeType volumeType)
{
    if (!isNormal)
        return false;

    result = ctrl->Wbt_GetMetaFileList(volumeType);

    if (0 == result.size())
        return false;

    return true;
}

// WBT : Get file inode info. for the given meta file.
bool
MetaFsWBTApi::GetMetaFileInode(std::string& fileName,
                MetaFileInodeDumpCxt& result, MetaVolumeType volumeType)
{
    if (!isNormal)
        return false;

    MetaFileInodeInfo* info = ctrl->Wbt_GetMetaFileInode(fileName, volumeType);

    if (nullptr == info)
        return false;

    result.inodeInfo = *info;

    delete info;

    return true;
}

void
MetaFsWBTApi::SetStatus(bool isNormal)
{
    this->isNormal = isNormal;
}
} // namespace pos
