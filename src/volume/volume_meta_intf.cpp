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

#include "src/volume/volume_meta_intf.h"

#include <rapidjson/document.h>
#include <string>
#include "src/metafs/include/metafs_service.h"
#include "src/helper/json/json_helper.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume.h"

namespace pos
{
int
VolumeMetaIntf::LoadVolumes(VolumeList& volList, std::string arrayName, int arrayID)
{
    std::string volFile = "vbr";
    uint32_t fileSize = 256 * 1024; // 256KB
    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);

    POS_EVENT_ID rc = metaFs->ctrl->CheckFileExist(volFile);
    if (EID(SUCCESS) != (int)rc)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED), "array_name: {}", arrayName);
        return EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED);
    }

    int fd = 0;
    rc = metaFs->ctrl->Open(volFile, fd);
    if (EID(SUCCESS) != (int)rc)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED), "array_name: {}", arrayName);
        return EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED);
    }

    char* rBuf = (char*)malloc(fileSize);
    memset(rBuf, 0, fileSize);

    // for partial read: metaFsMgr.io.Read(fd, byteOffset, dataChunkSize, rBuf);
    rc = metaFs->io->Read(fd, rBuf);
    metaFs->ctrl->Close(fd);

    if (EID(SUCCESS) != (int)rc)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_READ_FAILED),
            "array_name: {}", arrayName);
        free(rBuf);
        return EID(VOL_UNABLE_TO_LOAD_READ_FAILED);
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
                    std::string uuid = doc["volumes"][i]["uuid"].GetString();
                    uint64_t total = doc["volumes"][i]["total"].GetUint64();
                    uint64_t maxiops = doc["volumes"][i]["maxiops"].GetUint64();
                    uint64_t maxbw = doc["volumes"][i]["maxbw"].GetUint64();
                    VolumeBase* volume = new Volume(arrayName, arrayID, name, uuid, total, maxiops, maxbw);
                    volList.Add(volume, id);
                }
            }
        }
        catch (const std::exception& e)
        {
            POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_CONTENT_BROKEN),
                "reason: {}, array_name: {}", e.what(), arrayName);
            return EID(VOL_UNABLE_TO_LOAD_CONTENT_BROKEN);
        }
    }

    free(rBuf);
    return EID(SUCCESS);
}

int
VolumeMetaIntf::SaveVolumes(VolumeList& volList, std::string arrayName, int arrayID)
{
    std::string volFile = "vbr";
    uint32_t fileSize = 256 * 1024; // 256KB
    std::string contents = "";
    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);

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
                elem.SetAttribute(JsonAttribute("uuid", "\"" + vol->GetUuid() + "\""));
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

    POS_EVENT_ID rc = metaFs->ctrl->CheckFileExist(volFile);
    if (EID(SUCCESS) != (int)rc)
    {
        rc = metaFs->ctrl->Create(volFile, fileSize);
        if (EID(SUCCESS) != (int)rc)
        {
            POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_CREATION_FAILED),
                "array_name: {}", arrayName);
            return EID(VOL_UNABLE_TO_SAVE_CREATION_FAILED);
        }
    }

    int fd = 0;
    rc = metaFs->ctrl->Open(volFile, fd);
    if (EID(SUCCESS) != (int)rc)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_OPEN_FAILED),
            "array_name: {}", arrayName);
        return EID(VOL_UNABLE_TO_SAVE_OPEN_FAILED);
    }

    uint32_t contentsSize = contents.size();
    if (contentsSize >= fileSize)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_CONTENT_OVERFLOW),
            "array_name: {}", arrayName);
        return EID(VOL_UNABLE_TO_SAVE_CONTENT_OVERFLOW);
    }

    char* wBuf = (char*)malloc(fileSize);
    memset(wBuf, 0, fileSize);
    strncpy(wBuf, contents.c_str(), contentsSize);

    POS_EVENT_ID ioRC = metaFs->io->Write(fd, wBuf);

    metaFs->ctrl->Close(fd);

    if (EID(SUCCESS) != (int)ioRC)
    {
        free(wBuf);
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_WRITE_FAILED),
            "array_name: {}", arrayName);
        return EID(VOL_UNABLE_TO_SAVE_WRITE_FAILED);
    }

    free(wBuf);
    POS_TRACE_DEBUG(EID(SUCCESS), "SaveVolumes succeed");
    return EID(SUCCESS);
}

} // namespace pos
