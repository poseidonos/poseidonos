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

#include "map_content.h"

#include <string>

#include "map_flush_handler.h"
#include "mapper.h"
#include "src/include/memory.h"

namespace ibofos
{
MapContent::MapContent(int mapId)
: map(nullptr),
  mapIoHandler(nullptr),
  metaFile(nullptr),
  loaded(false)
{
    header.mapId = mapId;
}

MapContent::~MapContent(void)
{
    if (metaFile != nullptr)
    {
        if (metaFile->IsOpened())
        {
            metaFile->Close();
        }
        delete metaFile;
        metaFile = nullptr;
    }

    delete map;
    map = nullptr;
    delete mapIoHandler;
    mapIoHandler = nullptr;
}

void
MapContent::InitHeaderInfo(uint64_t numPages)
{
    header.SetMpageValidInfo(numPages, 0 /* numValidMpages*/);

    header.bitmap = new BitMap(numPages);
    header.bitmap->ResetBitmap();

    header.dirtyPages = new BitMap(numPages);
    header.dirtyPages->ResetBitmap();

    header.SetSize();

    header.isInitialized = true;
}

int
MapContent::CreateMapFile(void)
{
    if (header.isInitialized == false)
    {
        return -EID(MAPCONTENT_HEADER_NOT_INITIALIZED);
    }
    if (metaFile == nullptr)
    {
        metaFile = new FILESTORE(filename);
    }

    uint64_t fileSize = header.size + header.mpageSize * header.mpageData.numTotalMpages;

    int ret = metaFile->Create(fileSize);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(MFS_FILE_CREATE_FAILED),
            "Map file creation failed, fileName:{}", filename);
    }
    else if (ret == 0)
    {
        loaded = true;
    }

    return ret;
}

int
MapContent::DeleteMapFile(void)
{
    int ret = metaFile->Delete();
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(MFS_FILE_DELETE_FAILED), "MFS File:{} delete failed",
            filename);
        return ret;
    }
    delete metaFile;
    metaFile = nullptr;

    if (mapIoHandler != nullptr)
    {
        delete mapIoHandler;
        mapIoHandler = nullptr;
    }

    return ret;
}

bool
MapContent::DoesFileExist(void)
{
    return metaFile->DoesFileExist();
}

int
MapContent::LoadSync(int opt)
{
    int ret = 0;

    if (metaFile == nullptr)
    {
        metaFile = new FILESTORE(filename);
    }
    if (mapIoHandler == nullptr)
    {
        mapIoHandler = new MapIoHandler(map, &header);
    }

    if (metaFile->DoesFileExist() == false)
    {
        if (opt == CREATE_FILE_IF_NOT_EXIST)
        {
            ret = CreateMapFile();
            if (ret == 0)
            {
                ret = metaFile->Open();
                mapIoHandler->RegisterFile(metaFile);
                ret = mapIoHandler->SaveHeader();
            }
        }
        else
        {
            ret = -EID(TRIED_TO_LOAD_WITHOUT_MFSFILE);
        }
    }
    else
    {
        ret = metaFile->Open();
        mapIoHandler->RegisterFile(metaFile);
        ret = mapIoHandler->LoadFromMFS();
    }

    if (ret == 0)
    {
        loaded = true;
    }

    return ret;
}

int
MapContent::LoadAsync(AsyncLoadCallBack& cb)
{
    int ret = 0;

    if (metaFile == nullptr)
    {
        metaFile = new FILESTORE(filename);
    }
    ret = metaFile->Open();
    mapIoHandler->RegisterFile(metaFile);
    ret = mapIoHandler->AsyncLoad(cb);

    return ret;
}

int
MapContent::LoadAsyncEvent(AsyncLoadCallBack& cb)
{
    int ret = 0;

    if (metaFile == nullptr)
    {
        metaFile = new FILESTORE(filename);
    }
    ret = metaFile->Open();
    mapIoHandler->RegisterFile(metaFile);
    ret = mapIoHandler->AsyncLoadEvent(cb);

    return ret;
}

bool
MapContent::IsLoaded(void)
{
    return loaded;
}

int
MapContent::StoreSync(void)
{
    header.UpdateNumValidMpages();
    mapIoHandler->RegisterFile(metaFile);
    if (metaFile->IsOpened() == false)
    {
        metaFile->Open();
    }

    int ret = mapIoHandler->StoreToMFS();
    if (ret < 0)
    {
        IBOF_TRACE_WARN(EID(MFS_SYNCIO_ERROR), "mapId:{} StoreToMFS Failed @StoreSync",
            header.mapId);
    }
    else
    {
        IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "mapId:{} StoreToMFS Succeeded @StoreSync",
            header.mapId);
    }

    return ret;
}

int
MapContent::Unload(void)
{
    header.UpdateNumValidMpages();
    int ret = mapIoHandler->StoreToMFS();
    if (ret < 0)
    {
        IBOF_TRACE_WARN(EID(MFS_SYNCIO_ERROR), "mapId:{} StoreToMFS Failed @Unload",
            header.mapId);
    }
    else
    {
        IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "mapId:{} StoreToMFS Succeeded @Unload",
            header.mapId);
    }

    return ret;
}

int
MapContent::FileClose(void)
{
    int ret = metaFile->Close();
    if (ret == 0)
    {
        IBOF_TRACE_INFO(EID(MFS_INFO_MESSAGE), "File name:{} fd:{} has been closed",
            filename, metaFile->GetFd());
    }
    return ret;
}

int
MapContent::StartDirtyPageFlush(MpageList dirtyPages, EventSmartPtr callback)
{
    return mapIoHandler->FlushDirtyPages(dirtyPages, callback);
}

int
MapContent::Flush(EventSmartPtr callback)
{
    return mapIoHandler->Flush(callback);
}

uint32_t
MapContent::GetPageSize(void)
{
    return header.mpageSize;
}

int
MapContent::SetPageSize(StorageOpt storageOpt)
{
#ifdef IBOF_CONFIG_USE_MOCK_FS
    header.mpageSize = 4096;
#else
    MetaFilePropertySet prop;
    if (storageOpt == StorageOpt::NVRAM)
    {
        prop.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;
        prop.ioOpType = MDFilePropIoOpType::WriteDominant;
        prop.integrity = MDFilePropIntegrity::Lvl0_Disable;
    }

    header.mpageSize = metaFsMgr.util.EstimateAlignedFileIOSize(prop);
#endif
    return 0;
}

uint32_t
MapContent::GetEntriesPerPage(void)
{
    return header.entriesPerMpage;
}

std::string
MapContent::GetFileName(void)
{
    return filename;
}

int
MapContent::Init(uint64_t numPages)
{
    map = new Map(numPages, header.mpageSize);
    if (mapIoHandler == nullptr)
    {
        mapIoHandler = new MapIoHandler(map, &header);
    }

    return 0;
}

int
MapContent::Dump(std::string fname)
{
    MetaFileIntf* linuxFileToStore = new MockFileIntf(fname);
    int ret = linuxFileToStore->Create(0);
    linuxFileToStore->Open();

    if (ret == 0)
    {
        ret = mapIoHandler->SyncStore(linuxFileToStore);
    }

    linuxFileToStore->Close();
    delete linuxFileToStore;
    return ret;
}

int
MapContent::DumpLoad(std::string fname)
{
    MetaFileIntf* linuxFileFromLoad = new MockFileIntf(fname);
    int ret = linuxFileFromLoad->Open();

    if (ret == 0)
    {
        ret = mapIoHandler->SyncLoad(linuxFileFromLoad);
        ret = mapIoHandler->StoreToMFS();
    }

    linuxFileFromLoad->Close();
    delete linuxFileFromLoad;
    return ret;
}

int
MapContent::GetId(void)
{
    return header.mapId;
}

bool
MapContent::IsFileOpened(void)
{
    return metaFile->IsOpened();
}

int
MapContent::FileOpen(void)
{
    return metaFile->Open();
}

int
MapContent::GetNumMpageByLoadHeader(int volID,
    uint64_t& volSizeBytePageSizeAligned)
{
    std::unique_ptr<MetaFileIntf> file = std::make_unique<FILESTORE>(filename);
    int ret = 0;

    if (false == file->DoesFileExist())
    {
        IBOF_TRACE_WARN(EID(NO_BLOCKMAP_MFS_FILE), "No BlockMap MFS file, volumeID:{}",
            volID);
        return -EID(NO_BLOCKMAP_MFS_FILE);
    }

    if (false == file->IsOpened())
    {
        ret = file->Open();
        if (ret != 0)
        {
            IBOF_TRACE_WARN(EID(MFS_FILE_OPEN_FAILED), "MFS file open failed, volumeID:{}",
                volID);
            return -EID(MFS_FILE_OPEN_FAILED);
        }
    }

    MapHeader mapHeaderDummy;
    mapHeaderDummy.bitmap = new BitMap(1);
    MapIoHandler flushHandlerDummy(nullptr, &mapHeaderDummy);

    ret = file->IssueIO(MetaFsIoOpcode::Read, 0, sizeof(mapHeaderDummy.mpageData),
        (char*)&mapHeaderDummy.mpageData);
    if (ret != 0)
    {
        IBOF_TRACE_WARN(EID(MFS_FILE_READ_FAILED), "MFS file read failed, volumeID:{}",
            volID);
        return -EID(MFS_FILE_READ_FAILED);
    }

    if (header.entriesPerMpage == 0)
    {
        SetPageSize();
        header.entriesPerMpage = header.mpageSize / sizeof(VirtualBlkAddr);
    }
    volSizeBytePageSizeAligned = mapHeaderDummy.mpageData.numTotalMpages *
        header.entriesPerMpage *
        ibofos::BLOCK_SIZE;

    ret = file->Close();
    if (ret < 0)
    {
        IBOF_TRACE_WARN(EID(MFS_FILE_CLOSE_FAILED),
            "MFS file close failed, volumeID:{}", volID);
        return -EID(MFS_FILE_CLOSE_FAILED);
    }

    return 0;
}

} // namespace ibofos
