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

#include "src/metafs/include/metafs_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/mapper/reversemap/reversemap_manager.h"
#include "src/meta_file_intf/mock_file_intf.h"
#ifndef IBOF_CONFIG_USE_MOCK_FS
#include "src/metafs/metafs_file_intf.h"
#endif

#include <string>

namespace pos
{

ReverseMapManager::ReverseMapManager(VSAMapManager* ivsaMap, IStripeMap* istripeMap, IArrayInfo* iarrayInfo)
: mpageSize(0),
  numMpagesPerStripe(0),
  fileSizePerStripe(0),
  fileSizeWholeRevermap(0),
  revMapPacks(nullptr),
  revMapWholefile(nullptr),
  iVSAMap(ivsaMap),
  iStripeMap(istripeMap),
  iArrayInfo(iarrayInfo)
{
}

ReverseMapManager::~ReverseMapManager(void)
{
    if (revMapWholefile != nullptr)
    {
        revMapWholefile->Close();
        delete revMapWholefile;
        revMapWholefile = nullptr;
    }

    if (revMapPacks != nullptr)
    {
        delete [] revMapPacks;
        revMapPacks = nullptr;
    }
}

void
ReverseMapManager::Init(MapperAddressInfo& info)
{
    _SetPageSize();
    _SetNumMpages();

    fileSizeWholeRevermap = fileSizePerStripe * info.maxVsid;

    // Create MFS and Open the file for whole reverse map
    revMapWholefile = new FILESTORE("RevMapWhole", iArrayInfo->GetName());
    if (revMapWholefile->DoesFileExist() == false)
    {
        POS_TRACE_INFO(EID(REVMAP_FILE_SIZE), "fileSizePerStripe:{}  maxVsid:{}  fileSize:{} for RevMapWhole",
                        fileSizePerStripe, info.maxVsid, fileSizeWholeRevermap);

        int ret = revMapWholefile->Create(fileSizeWholeRevermap);
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(REVMAP_FILE_SIZE), "RevMapWhole file Create failed, ret:{}", ret);
            assert(false);
        }
    }
    revMapWholefile->Open();

    // Make ReverseMapPack:: objects by the number of WriteBuffer stripes
    revMapPacks = new ReverseMapPack[info.numWbStripes]();
    for (StripeId wbLsid = 0; wbLsid < info.numWbStripes; ++wbLsid)
    {
        std::string arrName = iArrayInfo->GetName();
        revMapPacks[wbLsid].Init(mpageSize, numMpagesPerStripe, revMapWholefile, arrName);
        revMapPacks[wbLsid].Init(VolumeServiceSingleton::Instance()->GetVolumeManager(arrName), wbLsid, iVSAMap, iStripeMap);
    }
}

void
ReverseMapManager::SetDoC(IArrayInfo* iarrayInfo)
{
    iArrayInfo = iarrayInfo;
}

void
ReverseMapManager::Dispose(void)
{
    if (revMapWholefile != nullptr)
    {
        if (revMapWholefile->IsOpened() == true)
        {
            revMapWholefile->Close();
        }
        delete revMapWholefile;
        revMapWholefile = nullptr;
    }
    if (revMapPacks != nullptr)
    {
        delete [] revMapPacks;
        revMapPacks = nullptr;
    }

}
//----------------------------------------------------------------------------//
ReverseMapPack*
ReverseMapManager::GetReverseMapPack(StripeId wbLsid)
{
    return &revMapPacks[wbLsid];
}

ReverseMapPack*
ReverseMapManager::AllocReverseMapPack(bool gcDest)
{
    ReverseMapPack* obj = new ReverseMapPack;
    std::string arrName = iArrayInfo->GetName();
    obj->Init(mpageSize, numMpagesPerStripe, revMapWholefile, arrName);
    if (gcDest == true)
    {
        obj->Init(VolumeServiceSingleton::Instance()->GetVolumeManager(arrName), UNMAP_STRIPE, iVSAMap, iStripeMap);
    }
    return obj;
}

uint64_t
ReverseMapManager::GetReverseMapPerStripeFileSize(void)
{
    return fileSizePerStripe;
}

uint64_t
ReverseMapManager::GetWholeReverseMapFileSize(void)
{
    return fileSizeWholeRevermap;
}

int
ReverseMapManager::LoadWholeReverseMap(char* pBuffer)
{
    int ret = revMapWholefile->IssueIO(MetaFsIoOpcode::Read, 0, fileSizeWholeRevermap, pBuffer);
    return ret;
}

int
ReverseMapManager::StoreWholeReverseMap(char* pBuffer)
{
    int ret = revMapWholefile->IssueIO(MetaFsIoOpcode::Write, 0, fileSizeWholeRevermap, pBuffer);
    return ret;
}

int
ReverseMapManager::_SetPageSize(StorageOpt storageOpt)
{
#ifdef IBOF_CONFIG_USE_MOCK_FS
    mpageSize = DEFAULT_REVMAP_PAGE_SIZE;
#else
    MetaFilePropertySet prop;
    if (storageOpt == StorageOpt::NVRAM)
    {
        prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
        prop.ioOpType = MetaFileDominant::WriteDominant;
        prop.integrity = MetaFileIntegrityType::Lvl0_Disable;
    }

    std::string arrayName = iArrayInfo->GetName();
    mpageSize = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName)->ctrl->EstimateAlignedFileIOSize(prop);
    if (mpageSize == 0)
    {
        POS_TRACE_CRITICAL(EID(REVMAP_GET_MFS_ALIGNED_IOSIZE_FAILURE), "MFS returned failure value");
        return -EID(REVMAP_GET_MFS_ALIGNED_IOSIZE_FAILURE);
    }
#endif
    POS_TRACE_INFO(EID(REVMAP_FILE_SIZE), "mPageSize for ReverseMap:{}", mpageSize);
    return 0;
}

int
ReverseMapManager::_SetNumMpages(void)
{
    const PartitionLogicalSize* udSize = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);

    uint32_t entriesPerNormalPage = (mpageSize / REVMAP_SECTOR_SIZE) * (REVMAP_SECTOR_SIZE / REVMAP_ENTRY_SIZE);
    uint32_t entriesPerFirstPage = entriesPerNormalPage - (REVMAP_SECTOR_SIZE / REVMAP_ENTRY_SIZE);

    if (udSize->blksPerStripe <= entriesPerFirstPage)
    {
        numMpagesPerStripe = 1;
    }
    else
    {
        numMpagesPerStripe = 1 + DivideUp(udSize->blksPerStripe - entriesPerFirstPage, entriesPerNormalPage);
    }

    // Set fileSize
    fileSizePerStripe = mpageSize * numMpagesPerStripe;

    POS_TRACE_INFO(EID(REVMAP_FILE_SIZE),
        "[ReverseMap Info] entriesPerNormalPage:{}  entriesPerFirstPage:{}  numMpagesPerStripe:{}  fileSizePerStripe:{}",
        entriesPerNormalPage, entriesPerFirstPage, numMpagesPerStripe,
        fileSizePerStripe);

    return 0;
}

} // namespace pos
