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
#include <atomic>
#include <string>
#include "src/metafs/include/metafs_service.h"
#include "src/helper/json/json_helper.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume.h"

using namespace std;

namespace pos
{
const std::string VolumeMetaIntf::FILE_NAME = "vbr";

int
VolumeMetaIntf::LoadVolumes(VolumeList& volList, const std::string& arrayName,
    const int arrayID, std::unique_ptr<MetaFsFileIntf> testFile)
{
    unique_ptr<MetaFsFileIntf> file = move(testFile);
    if (!file)
    {
        file = make_unique<MetaFsFileIntf>(FILE_NAME, arrayID, MetaFileType::General);
    }

    if (!file->DoesFileExist())
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED),
            "array_name: {}, array_id: {}",
            arrayName, arrayID);
        return EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED);
    }

    int rc = file->Open();
    if (EID(SUCCESS) != rc)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED),
            "error: {}, array_name: {}, array_id: {}",
            rc, arrayName, arrayID);
        return EID(VOL_UNABLE_TO_LOAD_OPEN_FAILED);
    }

    auto rBuf = _AllocateBuffer(FILE_SIZE);
    memset(rBuf.get(), 0, FILE_SIZE);

    rc = _ReadMetaFile(file.get(), (char*)rBuf.get());
    if (EID(SUCCESS) != rc)
    {
        POS_TRACE_ERROR(rc, "error: {}, array_name: {}, array_id: {}",
            rc, arrayName, arrayID);
        _CloseFile(move(file));
        return rc;
    }

    rc = _CloseFile(move(file));
    if (EID(SUCCESS) != rc)
    {
        return rc;
    }

    if (!string(rBuf.get()).empty())
    {
        return _FillVolumeList(volList, arrayName, arrayID, rBuf.get());
    }

    return EID(SUCCESS);
}

int
VolumeMetaIntf::SaveVolumes(VolumeList& volList, const string& arrayName,
    const int arrayID, std::unique_ptr<MetaFsFileIntf> testFile)
{
    string contents = "";
    unique_ptr<MetaFsFileIntf> file = move(testFile);
    if (nullptr == file)
    {
        file = make_unique<MetaFsFileIntf>(FILE_NAME, arrayID, MetaFileType::General);
    }

    if (false == file->DoesFileExist())
    {
        if (EID(SUCCESS) != file->Create(FILE_SIZE))
        {
            POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_CREATION_FAILED),
                "array_name: {}", arrayName);
            return EID(VOL_UNABLE_TO_SAVE_CREATION_FAILED);
        }
    }

    int rc = file->Open();
    if (EID(SUCCESS) != rc)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_OPEN_FAILED),
            "error: {}, array_name: {}, array_id: {}",
            rc, arrayName, arrayID);
        return EID(VOL_UNABLE_TO_SAVE_OPEN_FAILED);
    }

    contents = _CreateJsonFrom(volList);
    if (contents.size() >= FILE_SIZE)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_SAVE_CONTENT_OVERFLOW),
            "array_name: {}", arrayName);
        return EID(VOL_UNABLE_TO_SAVE_CONTENT_OVERFLOW);
    }

    auto wBuf = _AllocateBuffer(FILE_SIZE);
    memset(wBuf.get(), 0, FILE_SIZE);
    strncpy(wBuf.get(), contents.c_str(), contents.size());

    rc = _WriteBuffer(file.get(), (char*)wBuf.get());
    if (EID(SUCCESS) != rc)
    {
        POS_TRACE_ERROR(rc, "error: {}, array_name: {}, array_id: {}",
            rc, arrayName, arrayID);
        _CloseFile(move(file));
        return rc;
    }

    rc = _CloseFile(move(file));
    if (EID(SUCCESS) != rc)
    {
        return rc;
    }

    POS_TRACE_DEBUG(EID(SUCCESS), "SaveVolumes succeed");
    return EID(SUCCESS);
}

POS_EVENT_ID
VolumeMetaIntf::_FillVolumeList(VolumeList& volList, const std::string& arrayName,
    const int arrayID,const char *str)
{
    try
    {
        rapidjson::Document doc;
        doc.Parse<0>(str);
        if (doc.HasMember("volumes"))
        {
            for (rapidjson::SizeType i = 0; i < doc["volumes"].Size(); i++)
            {
                int id = doc["volumes"][i]["id"].GetInt();
                string name = doc["volumes"][i]["name"].GetString();
                string uuid = doc["volumes"][i]["uuid"].GetString();
                uint64_t total = doc["volumes"][i]["total"].GetUint64();
                uint64_t maxiops = doc["volumes"][i]["maxiops"].GetUint64();
                uint64_t maxbw = doc["volumes"][i]["maxbw"].GetUint64();
                uint64_t miniops = doc["volumes"][i]["miniops"].GetUint64();
                uint64_t minbw = doc["volumes"][i]["minbw"].GetUint64();
                uint32_t nsid = doc["volumes"][i]["nsid"].GetUint();
                DataAttribute dataAttribute = ((DataAttribute)doc["volumes"][i]["dataattribute"].GetInt());
                ReplicationRole volumeRole = ((ReplicationRole)doc["volumes"][i]["role"].GetInt());

                VolumeBase* volume = new Volume(arrayID, arrayName, dataAttribute, uuid,
                            name, total, nsid,
                            maxiops, miniops, maxbw, minbw,
                            volumeRole);
                volList.Add(volume, id);
            }
        }
    }
    catch (const exception& e)
    {
        POS_TRACE_ERROR(EID(VOL_UNABLE_TO_LOAD_CONTENT_BROKEN),
            "reason: {}, array_name: {}", e.what(), arrayName);
        return EID(VOL_UNABLE_TO_LOAD_CONTENT_BROKEN);
    }
    return EID(SUCCESS);
}

std::string
VolumeMetaIntf::_CreateJsonFrom(VolumeList& volList)
{
    if (0 == volList.Count())
    {
        return "";
    }

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
            elem.SetAttribute(JsonAttribute("name", "\"" + vol->GetVolumeName() + "\""));
            elem.SetAttribute(JsonAttribute("uuid", "\"" + vol->GetUuid() + "\""));
            elem.SetAttribute(JsonAttribute("id", to_string(vol->ID)));
            elem.SetAttribute(JsonAttribute("total", to_string(vol->GetTotalSize())));
            elem.SetAttribute(JsonAttribute("maxiops", to_string(vol->GetMaxIOPS())));
            elem.SetAttribute(JsonAttribute("maxbw", to_string(vol->GetMaxBW())));
            elem.SetAttribute(JsonAttribute("miniops", to_string(vol->GetMinIOPS())));
            elem.SetAttribute(JsonAttribute("minbw", to_string(vol->GetMinBW())));
            elem.SetAttribute(JsonAttribute("nsid", to_string(vol->GetNsid())));
            elem.SetAttribute(JsonAttribute("dataattribute", to_string(vol->GetDataAttribute())));
            elem.SetAttribute(JsonAttribute("role", to_string(vol->GetReplicationRole())));
            array.AddElement(elem);
        }
    }
    root.SetArray(array);
    return root.ToJson();
}


int
VolumeMetaIntf::_ReadMetaFile(MetaFsFileIntf* file, char* buf)
{
    std::atomic<bool> readDone;
    int readResult = 0;

    AsyncMetaFileIoCtx* writeRequest = new AsyncMetaFileIoCtx();
    writeRequest->SetIoInfo(MetaFsIoOpcode::Read, 0, FILE_SIZE, buf);
    writeRequest->SetFileInfo(file->GetFd(), file->GetIoDoneCheckFunc());
    writeRequest->SetCallback([&](auto arg) {
            readDone = true;
            readResult = arg->GetError();
            delete arg;
        });

    int rc = file->AsyncIO(writeRequest);
    if (EID(SUCCESS) != rc)
    {
        return rc;
    }

    while (!readDone)
    {
        usleep(1);
    }

    if (readResult)
    {
        return EID(VOL_UNABLE_TO_LOAD_READ_FAILED);
    }

    return rc;
}

int
VolumeMetaIntf::_WriteBuffer(MetaFsFileIntf* file, char* buf)
{
    std::atomic<bool> writeDone;
    int writeResult = 0;

    AsyncMetaFileIoCtx* writeRequest = new AsyncMetaFileIoCtx();
    writeRequest->SetIoInfo(MetaFsIoOpcode::Write, 0, FILE_SIZE, buf);
    writeRequest->SetFileInfo(file->GetFd(), file->GetIoDoneCheckFunc());
    writeRequest->SetCallback([&](auto arg) {
            writeDone = true;
            writeResult = arg->GetError();
            delete arg;
        });

    int rc = file->AsyncIO(writeRequest);

    if (EID(SUCCESS) != rc)
    {
        return rc;
    }

    while (!writeDone)
    {
        usleep(1);
    }

    if (writeResult)
    {
        return EID(VOL_UNABLE_TO_SAVE_WRITE_FAILED);
    }

    return rc;
}

int
VolumeMetaIntf::_CloseFile(unique_ptr<MetaFsFileIntf> file)
{
    if (file == nullptr)
    {
        POS_TRACE_ERROR(EID(VOL_INVALID_FILE_POINTER), "");
        assert(false);
    }

    int rc = file->Close();
    if (EID(SUCCESS) != rc)
    {
        POS_TRACE_ERROR(rc,
            "error:{}, fileName:{}, fd:{}",
            rc, file->GetFileName(), file->GetFd());
        return EID(VOL_UNABLE_TO_CLOSE_FILE);
    }

    return EID(SUCCESS);
}
} // namespace pos
