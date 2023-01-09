/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/lib/bitmap.h"
#include "src/metafs/common/meta_file_util.h"
#include "src/metafs/config/metafs_config.h"
#include "src/metafs/include/meta_file_context.h"
#include "src/metafs/util/metafs_spinlock.h"

namespace pos
{
using MetaVolTypeAndFdToFileName = std::unordered_map<
    std::pair<MetaVolumeType, FileDescriptorType>, std::string, PairHash>;
using MetaVolTypeAndFileNameToIndex = std::unordered_map<
    std::pair<MetaVolumeType, std::string>, uint32_t, PairHash>;

class MetaStorageSubsystem;
class MetaVolumeManager;
class MetaFileInodeInfo;

class MetaFileContextHandler
{
public:
    MetaFileContextHandler(const int arrayId,
        MetaStorageSubsystem* storage, MetaVolumeManager* volMgr);
    virtual ~MetaFileContextHandler(void);

    virtual void Initialize(const uint64_t signature);
    virtual MetaFileContext* GetFileContext(const FileDescriptorType fd,
        const MetaVolumeType type);
    virtual void RemoveFileContext(const FileDescriptorType fd,
        const MetaVolumeType type);
    virtual void AddFileContext(std::string& fileName,
        const FileDescriptorType fd, const MetaVolumeType type);

    // for test
    std::unique_ptr<BitMap> GetBitMap(void)
    {
        return std::move(usedCtxBitmap_);
    }
    // for test
    std::unique_ptr<MetaVolTypeAndFdToFileName> GetNameMap(void)
    {
        return std::move(nameMapByfd_);
    }
    // for test
    std::unique_ptr<MetaVolTypeAndFileNameToIndex> GetIndexMap(void)
    {
        return std::move(idxMapByName_);
    }
    // for test
    std::unique_ptr<std::vector<MetaFileContext>> GetFileContextList(void)
    {
        return std::move(ctxList_);
    }

private:
    void _UpdateFileContext(const uint32_t index, MetaFileInodeInfo* info);
    MetaFileInodeInfo* _GetFileInode(std::string& fileName, const MetaVolumeType type);

    std::unique_ptr<BitMap> usedCtxBitmap_;
    // (pair<MetaVolumeType, fd>, fileName)
    std::unique_ptr<MetaVolTypeAndFdToFileName> nameMapByfd_;
    // (pair<MetaVolumeType, fileName>, file index)
    std::unique_ptr<MetaVolTypeAndFileNameToIndex> idxMapByName_;
    std::unique_ptr<std::vector<MetaFileContext>> ctxList_;

    const uint32_t MAX_VOLUME_CNT = MetaFsConfig::MAX_VOLUME_CNT;
    MetaFsSpinLock iLock_;
    int arrayId_;
    uint64_t signature_;
    MetaStorageSubsystem* storage_;
    MetaVolumeManager* volMgr_;
};
} // namespace pos
