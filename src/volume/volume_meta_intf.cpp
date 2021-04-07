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

#include "src/volume/volume_meta_intf.h"

#include <rapidjson/document.h>

#include "mfs.h"
#include "src/helper/json_helper.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume.h"

namespace ibofos
{
int
VolumeMetaIntf::LoadVolumes(VolumeList& volList)
{
    std::string volFile = "vbr";
    uint32_t fileSize = 256 * 1024; // 256KB

    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
    mgmtRC = metaFsMgr.mgmt.CheckFileExist(volFile);
    if (!mgmtRC.IsSuccess())
    {
        return (int)IBOF_EVENT_ID::META_OPEN_FAIL;
    }

    mgmtRC = metaFsMgr.mgmt.Open(volFile);
    if (!mgmtRC.IsSuccess())
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::META_OPEN_FAIL, "Fail to open volume meta");
        return (int)IBOF_EVENT_ID::META_OPEN_FAIL;
    }

    uint32_t fd = mgmtRC.returnData;
    char* rBuf = (char*)malloc(fileSize);
    memset(rBuf, 0, fileSize);

    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;
    ioRC = metaFsMgr.io.Read(fd, rBuf); // for partial read: metaFsMgr.io.Read(fd, byteOffset, dataChunkSize, rBuf);
    metaFsMgr.mgmt.Close(fd);

    if (!ioRC.IsSuccess())
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::META_READ_FAIL, "Fail to read volume meta");
        free(rBuf);
        return (int)IBOF_EVENT_ID::META_READ_FAIL;
    }

    std::string contents = rBuf;
    if (contents != "")
    {
        try
        {
            rapidjson::Document doc;
            doc.Parse<0>(rBuf);
            if (doc.HasMember("volumes"))
            {
                for (rapidjson::SizeType i = 0; i < doc["volumes"].Size(); i++)
                {
                    int id = doc["volumes"][i]["id"].GetInt();
                    std::string name = doc["volumes"][i]["name"].GetString();
                    uint64_t total = doc["volumes"][i]["total"].GetUint64();
                    uint64_t maxiops = doc["volumes"][i]["maxiops"].GetUint64();
                    uint64_t maxbw = doc["volumes"][i]["maxbw"].GetUint64();
                    VolumeBase* volume = new Volume(name, total, maxiops, maxbw);
                    volList.Add(volume, id);
                }
            }
        }
        catch (const std::exception& e)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::META_CONTENT_BROKEN, "Volume meta broken {}", e.what());
            return (int)IBOF_EVENT_ID::META_CONTENT_BROKEN;
        }
    }

    free(rBuf);
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeMetaIntf::SaveVolumes(VolumeList& volList)
{
    std::string volFile = "vbr";
    uint32_t fileSize = 256 * 1024; // 256KB
    std::string contents = "";

    int vol_cnt = volList.Count();
    if (vol_cnt > 0)
    {
        JsonElement root("");
        JsonArray array("volumes");

        int idx = -1;
        while (true)
        {
            VolumeBase* vol = volList.Next(idx);
            if (vol == nullptr)
            {
                break;
            }
            if (vol->IsValid() == true)
            {
                JsonElement elem("");
                elem.SetAttribute(JsonAttribute("name", "\"" + vol->GetName() + "\""));
                elem.SetAttribute(JsonAttribute("id", std::to_string(vol->ID)));
                elem.SetAttribute(JsonAttribute("total", std::to_string(vol->TotalSize())));
                elem.SetAttribute(JsonAttribute("maxiops", std::to_string(vol->MaxIOPS())));
                elem.SetAttribute(JsonAttribute("maxbw", std::to_string(vol->MaxBW())));
                array.AddElement(elem);
            }
        }
        root.SetArray(array);
        contents = root.ToJson();
    }

    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
    mgmtRC = metaFsMgr.mgmt.CheckFileExist(volFile);
    if (!mgmtRC.IsSuccess())
    {
        mgmtRC = metaFsMgr.mgmt.Create(volFile, fileSize);
        if (!mgmtRC.IsSuccess())
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::META_CREATE_FAIL, "Fail to create meta file");
            return (int)IBOF_EVENT_ID::META_CREATE_FAIL;
        }
    }

    mgmtRC = metaFsMgr.mgmt.Open(volFile);
    if (!mgmtRC.IsSuccess())
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::META_OPEN_FAIL, "Fail to open meta file");
        return (int)IBOF_EVENT_ID::META_OPEN_FAIL;
    }

    uint32_t fd = mgmtRC.returnData;
    char* wBuf = (char*)malloc(fileSize);
    memset(wBuf, 0, fileSize);

    uint32_t contentsSize = contents.size();

    if (contentsSize >= fileSize)
    {
        free(wBuf);
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::SIZE_TOO_BIG, "Volume meta write buffer overflows");
        return (int)IBOF_EVENT_ID::SIZE_TOO_BIG;
    }

    strncpy(wBuf, contents.c_str(), contentsSize);

    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;
    ioRC = metaFsMgr.io.Write(fd, wBuf);

    metaFsMgr.mgmt.Close(fd);

    if (!ioRC.IsSuccess())
    {
        free(wBuf);
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::META_WRITE_FAIL, "Fail to write volume meta");
        return (int)IBOF_EVENT_ID::META_WRITE_FAIL;
    }

    free(wBuf);
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::SUCCESS, "SaveVolumes succeed");
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeMetaIntf::UpdateVolumeName(std::string oldName, std::string newName)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeMetaIntf::UpdateVolumeSize(std::string volName, uint64_t newVolSize)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

} // namespace ibofos
