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

#include "src/metafs/mai/meta_file_context_handler.h"

#include "src/metafs/config/metafs_config.h"
#include "src/metafs/log/metafs_log.h"
#include "src/metafs/mvm/meta_volume_manager.h"
#include "src/metafs/storage/mss.h"
#include "src/meta_file_intf/meta_file_intf.h"

namespace pos
{
MetaFileContextHandler::MetaFileContextHandler(const int arrayId,
    MetaStorageSubsystem* storage, MetaVolumeManager* volMgr)
: arrayId_(arrayId),
  signature_(UINT64_MAX),
  storage_(storage),
  volMgr_(volMgr)
{
    bitmap_ = std::make_unique<BitMap>(MAX_VOLUME_CNT);
    nameMapByfd_ = std::make_unique<MapToFindFileNameUsingMetaVolumeTypeAndFd>();
    idxMapByName_ = std::make_unique<MapToFindIndexUsingMetaVolumeTypeAndFileName>();
    ctxList_ = std::make_unique<std::vector<MetaFileContext>>(MAX_VOLUME_CNT);

    bitmap_->ResetBitmap();
}

MetaFileContextHandler::~MetaFileContextHandler(void)
{
}

void
MetaFileContextHandler::Initialize(const uint64_t signature)
{
    signature_ = signature;
}

MetaFileContext*
MetaFileContextHandler::GetFileContext(const FileDescriptorType fd, const MetaVolumeType type)
{
    SPIN_LOCK_GUARD_IN_SCOPE(iLock_);

    auto it = nameMapByfd_->find(make_pair(type, fd));

    if (it != nameMapByfd_->end())
    {
        auto result = idxMapByName_->find(make_pair(type, it->second));
        assert(result != idxMapByName_->end());
        return &ctxList_->at(result->second);
    }

    return nullptr;
}

void
MetaFileContextHandler::TryRemoveFileContext(const FileDescriptorType fd, const MetaVolumeType type)
{
    uint32_t index = 0;

    {
        SPIN_LOCK_GUARD_IN_SCOPE(iLock_);

        auto it1 = nameMapByfd_->find(make_pair(type, fd));
        if (it1 != nameMapByfd_->end())
        {
            std::string fileName = it1->second;
            auto it2 = idxMapByName_->find(make_pair(type, fileName));
            index = it2->second;

            nameMapByfd_->erase(it1);
            idxMapByName_->erase(it2);
            bitmap_->ClearBit(index);
        }
    }

    ctxList_->at(index).Reset();

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "FileContext is deallocated index:{}, arrayId:{}", index, arrayId_);
}

void
MetaFileContextHandler::AddFileContext(std::string& fileName,
    const FileDescriptorType fd, const MetaVolumeType type)
{
    uint32_t index = 0;

    {
        SPIN_LOCK_GUARD_IN_SCOPE(iLock_);

        auto key = make_pair(type, fd);
        if (nameMapByfd_->find(make_pair(type, fd)) != nameMapByfd_->end())
        {
            POS_TRACE_ERROR(EID(MFS_FILE_INFO_IS_ALREADY_IN_FILE_CONTEXT),
                "fileName:{}, fd:{}, volumeType:{}",
                fileName, fd, type);
            assert(false);
        }

        // get the position
        index = bitmap_->FindFirstZero();
        if (index >= MetaFsConfig::MAX_VOLUME_CNT)
        {
            POS_TRACE_ERROR(EID(MFS_NEED_MORE_CONTEXT_SLOT),
                "Metafile count:{}", index);
            assert(false);
        }

        bitmap_->SetBit(index);
        nameMapByfd_->insert({key, fileName});
        idxMapByName_->insert({make_pair(type, fileName), index});
    }

    // TODO (munseop.lim): use unique_ptr
    MetaFileInodeInfo* info = _GetFileInode(fileName, type);
    _UpdateFileContext(index, info);
    delete info;

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "FileContext is allocated index:{}, arrayId:{}", index, arrayId_);
}

void
MetaFileContextHandler::_UpdateFileContext(const uint32_t index, MetaFileInodeInfo* info)
{
    MetaFileContext* context = &ctxList_->at(index);
    context->Reset();
    context->isActivated = info->data.field.inUse;
    context->fileType = info->data.field.fileProperty.type;
    context->storageType = info->data.field.dataLocation;
    context->sizeInByte = info->data.field.fileByteSize;
    context->fileBaseLpn = info->data.field.extentMap[0].GetStartLpn();
    context->chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    context->extentsCount = info->data.field.extentCnt;
    context->CopyExtentsFrom(info->data.field.extentMap, context->extentsCount);
    context->signature = signature_;
    context->storage = storage_;
    assert(context->extentsCount != 0);
}

MetaFileInodeInfo*
MetaFileContextHandler::_GetFileInode(std::string& fileName, const MetaVolumeType type)
{
    MetaFsFileControlRequest reqMsg;
    POS_EVENT_ID rc;

    reqMsg.reqType = MetaFsFileControlType::GetFileInode;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId_;
    reqMsg.volType = type;

    rc = volMgr_->HandleNewRequest(reqMsg);

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.inodeInfoPointer;
    }

    return nullptr;
}
} // namespace pos
