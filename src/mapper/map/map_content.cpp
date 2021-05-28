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

#include "src/include/memory.h"
#include "src/metafs/include/metafs_service.h"
#include "src/mapper/map/map_content.h"
#include "src/mapper/map/map_flush_handler.h"

#include <string>

namespace pos
{
MapContent::MapContent(void)
: mapHeader(nullptr),
  map(nullptr),
  mapIoHandler(nullptr),
  metaFile(nullptr),
  metaFsService(nullptr),
  loaded(false)
{
}

MapContent::MapContent(MapHeader* mapHeader_, MetaFsService* metaFsService_)
: mapHeader(mapHeader_),
  map(nullptr),
  mapIoHandler(nullptr),
  metaFile(nullptr),
  metaFsService(metaFsService_),
  loaded(false)
{
}

MapContent::MapContent(int mapId)
: MapContent(new MapHeader, MetaFsServiceSingleton::Instance())
{
    mapHeader->SetMapId(mapId);
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
    if (map != nullptr)
    {
        delete map;
        map = nullptr;
    }
    if (mapIoHandler != nullptr)
    {
        delete mapIoHandler;
        mapIoHandler = nullptr;
    }
    if (mapHeader != nullptr)
    {
        delete mapHeader;
        mapHeader = nullptr;
    }
}

void
MapContent::InitHeaderInfo(uint64_t numPages)
{
    mapHeader->Init(numPages);
}

int
MapContent::CreateMapFile(void)
{
    if (mapHeader->IsInitialized() == false)
    {
        return -EID(MAPCONTENT_HEADER_NOT_INITIALIZED);
    }
    if (metaFile == nullptr)
    {
        metaFile = new FILESTORE(filename, arrayName);
    }

    uint64_t fileSize = mapHeader->GetSize() + mapHeader->GetMpageSize() * mapHeader->GetNumTotalMpages();

    int ret = metaFile->Create(fileSize);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_CREATE_FAILED), "Map file creation failed, fileName:{}", filename);
    }
    else if (ret == 0)
    {
        POS_TRACE_INFO(EID(MFS_START), "Map file created, fileName:{}", filename);
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
        POS_TRACE_ERROR(EID(MFS_FILE_DELETE_FAILED), "MFS File:{} delete failed",
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
        metaFile = new FILESTORE(filename, arrayName);
    }
    if (mapIoHandler == nullptr)
    {
        mapIoHandler = new MapIoHandler(map, mapHeader);
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
        metaFile = new FILESTORE(filename, arrayName);
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
        metaFile = new FILESTORE(filename, arrayName);
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
MapContent::StoreMap(void)
{
    mapIoHandler->RegisterFile(metaFile);
    if (metaFile->IsOpened() == false)
    {
        metaFile->Open();
    }

    return _Unload();
}

int
MapContent::_Unload(void)
{
    mapHeader->UpdateNumValidMpages();
    int ret = mapIoHandler->StoreToMFS();
    if (ret < 0)
    {
        POS_TRACE_WARN(EID(MFS_SYNCIO_ERROR), "mapId:{} StoreToMFS Failed @Unload", mapHeader->GetMapId());
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "mapId:{} StoreToMFS Succeeded @Unload", mapHeader->GetMapId());
    }

    return ret;
}

int
MapContent::FileClose(void)
{
    int ret = metaFile->Close();
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE), "File name:{} has been closed", filename);
    }
    return ret;
}

int
MapContent::FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback)
{
    return mapIoHandler->FlushDirtyPagesGiven(dirtyPages, callback);
}

int
MapContent::FlushTouchedPages(EventSmartPtr callback)
{
    return mapIoHandler->FlushTouchedPages(callback);
}

uint32_t
MapContent::GetPageSize(void)
{
    return mapHeader->GetMpageSize();
}

int
MapContent::SetPageSize(std::string aname, StorageOpt storageOpt)
{
#ifdef IBOF_CONFIG_USE_MOCK_FS
    mapHeader->SetMpageSize(4096);
#else
    MetaFilePropertySet prop;
    if (storageOpt == StorageOpt::NVRAM)
    {
        prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
        prop.ioOpType = MetaFileDominant::WriteDominant;
        prop.integrity = MetaFileIntegrityType::Lvl0_Disable;
    }

    MetaFs* metaFs = metaFsService->GetMetaFs(aname);
    if (metaFs == nullptr)
    {
        mapHeader->SetMpageSize(4096);
    }
    else
    {
        mapHeader->SetMpageSize(metaFs->ctrl->EstimateAlignedFileIOSize(prop));
    }
#endif
    return 0;
}

uint32_t
MapContent::GetEntriesPerPage(void)
{
    return mapHeader->GetEntriesPerMpage();
}

std::string
MapContent::GetFileName(void)
{
    return filename;
}

int
MapContent::Init(uint64_t numMpages)
{
    map = new Map(numMpages, mapHeader->GetMpageSize());
    if (mapIoHandler == nullptr)
    {
        mapIoHandler = new MapIoHandler(map, mapHeader);
    }

    return 0;
}

int
MapContent::Dump(std::string fname, std::string aname)
{
    MetaFileIntf* linuxFileToStore = new MockFileIntf(fname, aname);
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
MapContent::DumpLoad(std::string fname, std::string aname)
{
    MetaFileIntf* linuxFileFromLoad = new MockFileIntf(fname, aname);
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
    return mapHeader->GetMapId();
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

} // namespace pos
