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

#include "restore_manager.h"

#include <stdio.h>

#include <fstream>
#include <iostream>

#include "src/logger/logger.h"

namespace pos
{
RestoreManager::RestoreManager()
{
    commandProcessor = new CommandProcessor();
    jsonDocument = new Document();
    jsonDocument->SetObject();
    saveEnabled = false;
}

RestoreManager::~RestoreManager()
{
    if (jsonDocument != nullptr)
    {
        delete jsonDocument;
    }

    if (commandProcessor != nullptr)
    {
        delete commandProcessor;
    }
}

bool
RestoreManager::ArrayCreate(string name)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    if (!jsonDocument->HasMember("array"))
    {
        Value newArray(rapidjson::kArrayType);
        jsonDocument->AddMember("array", newArray, jsonDocument->GetAllocator());
    }

    for (auto& arr : (*jsonDocument)["array"].GetArray())
    {
        if (arr["array-name"].GetString() == name)
        {
            POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_DUPLICATE), "The same array already exists. The current CLI command was not saved.");
            return false;
        }
    }

    Value arrayObject(rapidjson::kObjectType);
    arrayObject.AddMember("array-name", name, jsonDocument->GetAllocator());
    arrayObject.AddMember("mount", false, jsonDocument->GetAllocator());
    arrayObject.AddMember("enable-write-through", false, jsonDocument->GetAllocator());
    arrayObject.AddMember("traddr", "", jsonDocument->GetAllocator());

    (*jsonDocument)["array"].GetArray().PushBack(arrayObject, jsonDocument->GetAllocator());

    return _WriteJson();
}

bool
RestoreManager::ArrayDelete(string name)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    bool arrayErased = false;
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            if (arr["array-name"].GetString() == name)
            {
                (*jsonDocument)["array"].Erase(&arr);
                arrayErased = true;
                break;
            }
        }
    }
    if (!arrayErased)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no array to delete.");
        return false;
    }

    return _WriteJson();
}

bool
RestoreManager::SetArrayMountState(string name, bool isMount, bool isWt, string trAddr)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    bool arrayMount = false;
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            if (arr["array-name"].GetString() == name)
            {
                arr["mount"].SetBool(isMount);
                arr["enable-write-through"].SetBool(isWt);
                arr["traddr"].SetString(StringRef(trAddr.c_str()));
                arrayMount = true;
                break;
            }
        }
    }
    if (!arrayMount)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no array to mount.");
        return false;
    }

    return _WriteJson();
}

bool
RestoreManager::VolumeCreate(string arrayName, string volName, int32_t nsid)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    if (!jsonDocument->HasMember("array"))
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no array to create volume.");
        return false;
    }

    bool volumeCreated = false;
    for (auto& arr : (*jsonDocument)["array"].GetArray())
    {
        if (arr["array-name"].GetString() == arrayName)
        {
            if (!arr.HasMember("volume"))
            {
                Value newVolumeArray(rapidjson::kArrayType);
                arr.AddMember("volume", newVolumeArray, jsonDocument->GetAllocator());
            }

            for (auto& volArr : arr["volume"].GetArray())
            {
                if (volArr["volume-name"] == volName)
                {
                    POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_DUPLICATE), "The same volume already exists. The current CLI command was not saved.");
                    return false;
                }
            }

            Value volumeObject(rapidjson::kObjectType);
            volumeObject.AddMember("volume-name", volName, jsonDocument->GetAllocator());
            volumeObject.AddMember("mount", false, jsonDocument->GetAllocator());
            volumeObject.AddMember("subnqn", "", jsonDocument->GetAllocator());
            volumeObject.AddMember("nsid", nsid, jsonDocument->GetAllocator());
            arr["volume"].GetArray().PushBack(volumeObject, jsonDocument->GetAllocator());
            volumeCreated = true;
            break;
        }
    }

    if (!volumeCreated)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no array to create volume.");
        return false;
    }
    return _WriteJson();
}

bool
RestoreManager::VolumeDelete(string arrayName, string volName)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    bool volumeErased = false;
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            if (arr["array-name"].GetString() == arrayName)
            {
                if (arr.HasMember("volume"))
                {
                    for (auto& volArr : arr["volume"].GetArray())
                    {
                        if (volArr["volume-name"] == volName)
                        {
                            arr["volume"].Erase(&volArr);
                            volumeErased = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!volumeErased)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no volume to delete.");
        return false;
    }

    return _WriteJson();
}

bool
RestoreManager::SetVolumeMountState(string arrayName, string volName, bool isMount, string subNqn, int32_t nsid)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    bool volumeMount = false;
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            if (arr["array-name"].GetString() == arrayName)
            {
                if (arr.HasMember("volume"))
                {
                    for (auto& volArr : arr["volume"].GetArray())
                    {
                        if (volArr["volume-name"] == volName)
                        {
                            volArr["mount"].SetBool(isMount);
                            volArr["subnqn"].SetString(StringRef(subNqn.c_str()));
                            volArr["nsid"].SetInt(nsid);
                            volumeMount = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!volumeMount)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no volume to mount.");
        return false;
    }

    return _WriteJson();
}

bool
RestoreManager::VolumeRename(string arrayName, string volOldName, string volNewName)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    bool volumeRename = false;
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            if (arr["array-name"].GetString() == arrayName)
            {
                if (arr.HasMember("volume"))
                {
                    for (auto& volArr : arr["volume"].GetArray())
                    {
                        if (volArr["volume-name"] == volOldName)
                        {
                            volArr["volume-name"].SetString(StringRef(volNewName.c_str()));
                            volumeRename = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!volumeRename)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no volume to rename.");
        return false;
    }

    return _WriteJson();
}

bool
RestoreManager::TransportCreate(string trType, uint32_t bufCacheSize, uint32_t numSharedBuf, uint32_t ioUnitSize)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    std::transform(trType.begin(), trType.end(), trType.begin(), ::tolower);

    if (!jsonDocument->HasMember("transport"))
    {
        Value newTransport(rapidjson::kObjectType);
        jsonDocument->AddMember("transport", newTransport, jsonDocument->GetAllocator());
    }

    if ((*jsonDocument)["transport"].HasMember(trType.c_str()))
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_DUPLICATE), "The same transport already exists.");
        return false;
    }

    Value tranportObject(rapidjson::kObjectType);
    tranportObject.AddMember("buf-cache-size", bufCacheSize, jsonDocument->GetAllocator());
    tranportObject.AddMember("num-shared-buf", numSharedBuf, jsonDocument->GetAllocator());
    tranportObject.AddMember("io-unit-size", ioUnitSize, jsonDocument->GetAllocator());
    (*jsonDocument)["transport"].AddMember(StringRef(trType.c_str()), tranportObject, jsonDocument->GetAllocator());

    return _WriteJson();
}

bool
RestoreManager::DeviceCreate(string name, string type, uint32_t blockSize, uint32_t numBlocks, uint32_t numa)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    if (!jsonDocument->HasMember("device"))
    {
        Value newDevice(rapidjson::kObjectType);
        jsonDocument->AddMember("device", newDevice, jsonDocument->GetAllocator());
    }

    if (!(*jsonDocument)["device"].HasMember(type.c_str()))
    {
        Value newType(rapidjson::kArrayType);
        (*jsonDocument)["device"].AddMember(StringRef(type.c_str()), newType, jsonDocument->GetAllocator());
    }

    for (auto& arr : (*jsonDocument)["device"][type.c_str()].GetArray())
    {
        if (arr["device-name"].GetString() == name)
        {
            POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_DUPLICATE), "The same device already exists. The current CLI command was not saved.");
            return false;
        }
    }

    Value deviceObject(rapidjson::kObjectType);
    deviceObject.AddMember("device-name", name, jsonDocument->GetAllocator());
    deviceObject.AddMember("block-size", blockSize, jsonDocument->GetAllocator());
    deviceObject.AddMember("num-blocks", numBlocks, jsonDocument->GetAllocator());
    deviceObject.AddMember("numa", numa, jsonDocument->GetAllocator());

    (*jsonDocument)["device"][type.c_str()].GetArray().PushBack(deviceObject, jsonDocument->GetAllocator());

    return _WriteJson();
}

bool
RestoreManager::SubsystemCreate(string subnqn, string serialNumber, string modelNumber, uint32_t maxNamespaces, bool allowAnyHost, bool anaReporting)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    if (!jsonDocument->HasMember("subsystem"))
    {
        Value newSubsystem(rapidjson::kArrayType);
        jsonDocument->AddMember("subsystem", newSubsystem, jsonDocument->GetAllocator());
    }

    for (auto& arr : (*jsonDocument)["subsystem"].GetArray())
    {
        if (arr["subnqn"].GetString() == subnqn)
        {
            POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_DUPLICATE), "The same subsystem already exists. The current CLI command was not saved.");
            return false;
        }
    }

    Value subsystenObject(rapidjson::kObjectType);
    subsystenObject.AddMember("subnqn", subnqn, jsonDocument->GetAllocator());
    subsystenObject.AddMember("serial-number", serialNumber, jsonDocument->GetAllocator());
    subsystenObject.AddMember("model-number", modelNumber, jsonDocument->GetAllocator());
    subsystenObject.AddMember("max-namespaces", maxNamespaces, jsonDocument->GetAllocator());
    subsystenObject.AddMember("allow-any-host", allowAnyHost, jsonDocument->GetAllocator());
    subsystenObject.AddMember("ana-reporting", anaReporting, jsonDocument->GetAllocator());

    (*jsonDocument)["subsystem"].GetArray().PushBack(subsystenObject, jsonDocument->GetAllocator());

    return _WriteJson();
}

bool
RestoreManager::SubsystemDelete(string subnqn)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    bool subsystemErased = false;
    if (jsonDocument->HasMember("subsystem"))
    {
        for (auto& arr : (*jsonDocument)["subsystem"].GetArray())
        {
            if (arr["subnqn"].GetString() == subnqn)
            {
                (*jsonDocument)["subsystem"].Erase(&arr);
                subsystemErased = true;
                break;
            }
        }
    }
    if (!subsystemErased)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no subsystem to delete.");
        return false;
    }

    return _WriteJson();
}

bool
RestoreManager::ListenerAdd(string subnqn, string trType, string trAddr, string trSvcid)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    if (!jsonDocument->HasMember("subsystem"))
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no subsystem to add listener. The current CLI command was not saved.");
        return false;
    }

    std::transform(trType.begin(), trType.end(), trType.begin(), ::tolower);

    bool listenerAdded = false;
    for (auto& arr : (*jsonDocument)["subsystem"].GetArray())
    {
        if (arr["subnqn"].GetString() == subnqn)
        {
            if (arr.HasMember("listener"))
            {
                POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_DUPLICATE), "This subsystem already has listener. The current CLI command was not saved.");
                return false;
            }
            else
            {
                Value listenerObject(rapidjson::kObjectType);
                listenerObject.AddMember("trtype", trType, jsonDocument->GetAllocator());
                listenerObject.AddMember("traddr", trAddr, jsonDocument->GetAllocator());
                listenerObject.AddMember("trsvcid", trSvcid, jsonDocument->GetAllocator());
                arr.AddMember("listener", listenerObject, jsonDocument->GetAllocator());
                listenerAdded = true;
                break;
            }
        }
    }

    if (!listenerAdded)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no subsystem to add listener.");
        return false;
    }
    return _WriteJson();
}

bool
RestoreManager::ListenerRemove(string subnqn, string trType, string trAddr, string trSvcid)
{
    if (!_CheckJsonWrite())
    {
        return false;
    }

    std::transform(trType.begin(), trType.end(), trType.begin(), ::tolower);

    bool listenerRemoved = false;
    if (jsonDocument->HasMember("subsystem"))
    {
        for (auto& arr : (*jsonDocument)["subsystem"].GetArray())
        {
            if ((arr["subnqn"].GetString() == subnqn) && arr.HasMember("listener"))
            {
                if ((arr["listener"]["trtype"] == trType) && (arr["listener"]["traddr"] == trAddr) && (arr["listener"]["trsvcid"] == trSvcid))
                {
                    arr.RemoveMember("listener");
                    listenerRemoved = true;
                    break;
                }
            }
        }
    }
    if (!listenerRemoved)
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_OBJECT_EMPTY), "There is no listener in subsystem({}) to remove.", subnqn);
        return false;
    }

    return _WriteJson();
}

void
RestoreManager::ClearRestoreState()
{
    jsonDocument->RemoveAllMembers();
    if (_WriteJson())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_WRITE_SUCCESS), "Clear saved state.");
    }
    else
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_WRITE_FAIL), "Failed to clear saved state.");
    }
}

void
RestoreManager::Print()
{
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    jsonDocument->Accept(writer);
    cout << buffer.GetString() << std::endl;
}

void
RestoreManager::EnableStateSave()
{
    saveEnabled = true;
    POS_TRACE_INFO(EID(RESTORE_MSG), "Saving state enabled.");
}

bool
RestoreManager::_ReadJson(void)
{
    jsonDocument->RemoveAllMembers();

    ifstream ifs(restore_file);
    if (!ifs.is_open())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_READ_FAIL), "Failed to open restore.json file. We'll create new one if possible.");
        return true;
    }

    IStreamWrapper isw(ifs);
    jsonDocument->ParseStream(isw);
    if (jsonDocument->HasParseError())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_READ_FAIL), "Failed to parse restore.json file. Check the file is in JSON format");
        return false;
    }

    // Todo: Check each value format is correct

    if (!jsonDocument->IsObject())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_READ_FAIL), "Failed to parse restore.json. Restore.json must be object.");
        return false;
    }
    return true;
}

bool
RestoreManager::_WriteJson(void)
{
    ofstream ofs(restore_file);
    if (!ofs.is_open())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_WRITE_FAIL), "Failed to open(create) restore.json file. The current CLI command was not saved.");
        return false;
    }
    OStreamWrapper osw(ofs);
    PrettyWriter<OStreamWrapper> writer(osw);
    jsonDocument->Accept(writer);
    return true;
}

bool
RestoreManager::Restore()
{
    if (!_ReadJson())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_READ_FAIL), "There was an error opening the restore.json file.");
        return false;
    }
    if (jsonDocument->ObjectEmpty())
    {
        return true;
    }
    POS_TRACE_INFO(EID(RESTORE_MSG), "Checked for the correct files needed for restore(/etc/pos/restore.json). Starting to restore state");

    if (!_TransportRestore())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_FAIL), "Failed to restore transport. Saving state is disabled.");
        return false;
    }

    if (!_SubsystemRestore())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_FAIL), "Failed to restore transport. Saving state is disabled.");
        return false;
    }

    if (!_DeviceRestore())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_FAIL), "Failed to restore device. Saving state is disabled.");
        return false;
    }

    _ScanDevice();

    if (!_ArrayRestore())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_FAIL), "Failed to restore array. Saving state is disabled.");
        return false;
    }

    if (!_VolumeRestore())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_FAIL), "Failed to restore volume. Saving state is disabled.");
        return false;
    }

    POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "The state restore succeeded.");
    return true;
}

bool
RestoreManager::_ArrayRestore(void)
{
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            if (arr["mount"].GetBool() == true)
            {
                MountArrayRequest* request = new MountArrayRequest;
                grpc_cli::MountArrayRequest_Param* requestParam = new grpc_cli::MountArrayRequest_Param;
                requestParam->set_name(arr["array-name"].GetString());
                requestParam->set_enablewritethrough(arr["enable-write-through"].GetBool());
                requestParam->set_targetaddress(arr["traddr"].GetString());
                request->set_allocated_param(requestParam);
                MountArrayResponse response;
                grpc::Status status = commandProcessor->ExecuteMountArrayCommand(request, &response);
                delete request;
                if (response.result().status().code() != EID(SUCCESS))
                {
                    return false;
                }
                else
                {
                    POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "Array({}) is mounted.", arr["array-name"].GetString());
                }
            }
        }
    }
    return true;
}

bool
RestoreManager::_VolumeRestore(void)
{
    if (jsonDocument->HasMember("array"))
    {
        for (auto& arr : (*jsonDocument)["array"].GetArray())
        {
            string arrayName = arr["array-name"].GetString();
            if (arr.HasMember("volume"))
            {
                for (auto& volArr : arr["volume"].GetArray())
                {
                    string volumeName = volArr["volume-name"].GetString();
                    string subNqn = volArr["subnqn"].GetString();
                    if (volArr["mount"] == true)
                    {
                        MountVolumeRequest* request = new MountVolumeRequest;
                        grpc_cli::MountVolumeRequest_Param* requestParam = new grpc_cli::MountVolumeRequest_Param;
                        requestParam->set_name(volumeName);
                        requestParam->set_array(arrayName);
                        requestParam->set_subnqn(volArr["subnqn"].GetString());
                        requestParam->set_nsid(volArr["nsid"].GetInt());
                        request->set_allocated_param(requestParam);
                        MountVolumeResponse response;
                        grpc::Status status = commandProcessor->ExecuteMountVolumeCommand(request, &response);
                        delete request;
                        if (response.result().status().code() != EID(SUCCESS))
                        {
                            return false;
                        }
                        else
                        {
                            POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "Volume({}) is mounted on array({})", volumeName, arrayName);
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool
RestoreManager::_TransportRestore(void)
{
    if (jsonDocument->HasMember("transport"))
    {
        for (auto& obj : (*jsonDocument)["transport"].GetObject())
        {
            CreateTransportRequest* request = new CreateTransportRequest;
            grpc_cli::CreateTransportRequest_Param* requestParam = new grpc_cli::CreateTransportRequest_Param;
            requestParam->set_transporttype(obj.name.GetString());
            requestParam->set_bufcachesize(obj.value["buf-cache-size"].GetUint());
            requestParam->set_numsharedbuf(obj.value["num-shared-buf"].GetUint());

            request->set_allocated_param(requestParam);
            CreateTransportResponse response;
            grpc::Status status = commandProcessor->ExecuteCreateTransportCommand(request, &response);
            delete request;
            if (response.result().status().code() != EID(SUCCESS))
            {
                return false;
            }
            else
            {
                POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "Transport({}) is created.", obj.name.GetString());
            }
        }
    }
    return true;
}

bool
RestoreManager::_DeviceRestore(void)
{
    if (jsonDocument->HasMember("device"))
    {
        if ((*jsonDocument)["device"].HasMember("uram"))
        {
            for (auto& arr : (*jsonDocument)["device"]["uram"].GetArray())
            {
                CreateDeviceRequest* request = new CreateDeviceRequest;
                grpc_cli::CreateDeviceRequest_Param* requestParam = new grpc_cli::CreateDeviceRequest_Param;
                requestParam->set_name(arr["device-name"].GetString());
                requestParam->set_blocksize(arr["block-size"].GetUint());
                requestParam->set_numblocks(arr["num-blocks"].GetUint());
                requestParam->set_numa(arr["numa"].GetUint());
                request->set_allocated_param(requestParam);
                CreateDeviceResponse response;
                grpc::Status status = commandProcessor->ExecuteCreateDeviceCommand(request, &response);
                delete request;
                if (response.result().status().code() != EID(SUCCESS))
                {
                    return false;
                }
                else
                {
                    POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "Device({}) is created.", arr["device-name"].GetString());
                }
            }
        }
    }
    return true;
}

bool
RestoreManager::_SubsystemRestore(void)
{
    if (jsonDocument->HasMember("subsystem"))
    {
        for (auto& arr : (*jsonDocument)["subsystem"].GetArray())
        {
            CreateSubsystemRequest* request = new CreateSubsystemRequest;
            grpc_cli::CreateSubsystemRequest_Param* requestParam = new grpc_cli::CreateSubsystemRequest_Param;
            request->set_command("CREATESUBSYSTEM");
            requestParam->set_nqn(arr["subnqn"].GetString());
            requestParam->set_serialnumber(arr["serial-number"].GetString());
            requestParam->set_modelnumber(arr["model-number"].GetString());
            requestParam->set_maxnamespaces(arr["max-namespaces"].GetUint());
            requestParam->set_allowanyhost(arr["allow-any-host"].GetBool());
            requestParam->set_anareporting(arr["ana-reporting"].GetBool());
            request->set_allocated_param(requestParam);
            CreateSubsystemResponse response;
            grpc::Status status = commandProcessor->ExecuteCreateSubsystemCommand(request, &response);
            delete request;
            if (response.result().status().code() != EID(SUCCESS))
            {
                return false;
            }
            else
            {
                POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "Subsystem({}) is created.", arr["subnqn"].GetString());
            }

            if (arr.HasMember("listener"))
            {
                AddListenerRequest* request = new AddListenerRequest;
                grpc_cli::AddListenerRequest_Param* requestParam = new grpc_cli::AddListenerRequest_Param;
                requestParam->set_subnqn(arr["subnqn"].GetString());
                requestParam->set_transporttype(arr["listener"]["trtype"].GetString());
                requestParam->set_targetaddress(arr["listener"]["traddr"].GetString());
                requestParam->set_transportserviceid(arr["listener"]["trsvcid"].GetString());
                request->set_allocated_param(requestParam);
                AddListenerResponse response;
                grpc::Status status = commandProcessor->ExecuteAddListenerCommand(request, &response);
                delete request;
                if (response.result().status().code() != EID(SUCCESS))
                {
                    return false;
                }
                else
                {
                    POS_TRACE_INFO(EID(RESTORE_JSON_SUCCESS), "Listener({}) is added to {})", arr["listener"]["trtype"].GetString(), arr["subnqn"].GetString());
                }
            }
        }
    }
    return true;
}

bool
RestoreManager::_ScanDevice(void)
{
    ScanDeviceRequest* request = new ScanDeviceRequest;
    ScanDeviceResponse response;
    grpc::Status status = commandProcessor->ExecuteScanDeviceCommand(request, &response);
    delete request;

    return true;
}

bool
RestoreManager::_CheckJsonWrite(void)
{
    if (!saveEnabled)
    {
        return false;
    }

    if (!_ReadJson())
    {
        POS_TRACE_INFO(EID(RESTORE_JSON_READ_FAIL), "There was an error opening the restore.json file. The current CLI command was not saved.");
        return false;
    }

    return true;
}

} // namespace pos
