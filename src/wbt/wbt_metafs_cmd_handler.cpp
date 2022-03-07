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

#include <string>
#include <vector>
#include "src/metafs/include/metafs_service.h"
#include "wbt_metafs_cmd_handler.h"
#include "src/spdk_wrapper/spdk.h"
#include "src/metafs/tool/fio/meta_scheduler.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/metafs/common/meta_file_util.h"

#define RESULT_SUCCESS 0
#define RESULT_FAILURE -1

namespace pos
{
int
WbtMetafsCmdHandler::DumpFilesList(Args argv)
{
    if (argv.size() < 3)
    {
        std::cout << "mfs_dump_files_list Command. Too Few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    int res = RESULT_SUCCESS;
    std::string parsedJson;
    std::string arrayName = argv["array"].get<std::string>();
    std::string outFile = argv["output"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    std::vector<MetaFileInfoDumpCxt> result;
    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    if ((nullptr == metaFs) ||
        (!metaFs->wbt->GetMetaFileList(result, volumeType)))
    {
        return RESULT_FAILURE;
    }

    JsonElement element("");
    JsonArray arrayElement("filesInfoList");

    for (unsigned int i = 0; i < result.size(); i++)
    {
        _DumpFilesInfoToJsonElement(result[i], arrayElement);
    }

    element.SetArray(arrayElement);
    parsedJson = element.ToJson();
    parsedJson += "\n";

    if (_WriteBufferInFile(outFile, parsedJson.c_str(), parsedJson.length()) == RESULT_FAILURE)
    {
        return RESULT_FAILURE;
    }

    return res;
}

int
WbtMetafsCmdHandler::CreateFile(Args argv)
{
    if (argv.size() < 7)
    {
        std::cout << "mfs_create_file Command. Too Few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    MetaFilePropertySet fileProperty = MetaFilePropertySet();

    std::string fileName = argv["name"].get<std::string>();
    std::string arrayName = argv["array"].get<std::string>();
    uint32_t fileSizeBytes = stoi(argv["size"].get<std::string>());
    int integrityType = stoi(argv["integrity"].get<std::string>());
    int ioAccPatternType = stoi(argv["access"].get<std::string>());
    int ioOpType = stoi(argv["operation"].get<std::string>());
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    fileProperty.integrity = static_cast<MetaFileIntegrityType>(integrityType);
    fileProperty.ioAccPattern = static_cast<MetaFileAccessPattern>(ioAccPatternType);
    fileProperty.ioOpType = static_cast<MetaFileDominant>(ioOpType);

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);
    if (nullptr == metaFs)
        return RESULT_FAILURE;

    POS_EVENT_ID rc = metaFs->ctrl->Create(fileName, fileSizeBytes, fileProperty, volumeType);
    if (rc != POS_EVENT_ID::SUCCESS)
        return RESULT_FAILURE;

    return RESULT_SUCCESS;
}

int
WbtMetafsCmdHandler::OpenFile(Args argv)
{
    if (argv.size() < 3)
    {
        std::cout << "mfs_open_file Command. Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    int fd = 0;

    std::string fileName = argv["name"].get<std::string>();
    std::string arrayName = argv["array"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);
    if (nullptr == metaFs)
        return RESULT_FAILURE;

    POS_EVENT_ID rc = metaFs->ctrl->Open(fileName, fd, volumeType);
    if (rc != POS_EVENT_ID::SUCCESS)
        return RESULT_FAILURE;

    return fd;
}

int
WbtMetafsCmdHandler::CloseFile(Args argv)
{
    if (argv.size() < 3)
    {
        std::cout << "mfs_close_file Command. Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    int fd = stoi(argv["fd"].get<std::string>());
    std::string arrayName = argv["array"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);
    if (nullptr == metaFs)
        return RESULT_FAILURE;

    POS_EVENT_ID rc = metaFs->ctrl->Close(fd, volumeType);
    if (rc != POS_EVENT_ID::SUCCESS)
        return RESULT_FAILURE;

    return RESULT_SUCCESS;
}

int
WbtMetafsCmdHandler::ReadFile(Args argv)
{
    if (argv.size() < 6)
    {
        std::cout << "mfs_read_file Command. Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    std::string arrayName = argv["array"].get<std::string>();
    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);

    if (nullptr == metaFs)
        return RESULT_FAILURE;

    bool retVal = true;
    int fd = stoi(argv["fd"].get<std::string>());
    uint32_t byteOffset = stoi(argv["offset"].get<std::string>());
    uint32_t byteSize = stoi(argv["count"].get<std::string>());
    char* buffer = new char[byteSize];
    std::string outFile = argv["output"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
    {
        if (buffer != nullptr)
            delete[] buffer;

        return RESULT_FAILURE;
    }

    MetaStorageType storage = MetaFileUtil::ConvertToMediaType(volumeType);

    if (buffer == nullptr)
    {
        std::cout << "[Error] Buffer allocation Failed." << std::endl;
        return RESULT_FAILURE;
    }

    memset(buffer, 0, byteSize);

    POS_EVENT_ID rc = metaFs->io->Read(fd, byteOffset, byteSize, (void*)buffer, storage);
    if (rc != POS_EVENT_ID::SUCCESS || _WriteBufferInFile(outFile, buffer, byteSize) != RESULT_SUCCESS)
    {
        retVal = false;
    }

    if (buffer != nullptr)
        delete[] buffer;

    if (retVal == false)
        return RESULT_FAILURE;

    return RESULT_SUCCESS;
}

int
WbtMetafsCmdHandler::WriteFile(Args argv)
{
    if (argv.size() < 6)
    {
        std::cout << "mfs_write_file Command.Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    std::string arrayName = argv["array"].get<std::string>();
    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);

    if (nullptr == metaFs)
        return RESULT_FAILURE;

    POS_EVENT_ID rc;
    int fd = stoi(argv["fd"].get<std::string>());
    uint32_t byteOffset = stoi(argv["offset"].get<std::string>());
    uint32_t byteSize = stoi(argv["count"].get<std::string>());
    char* buffer = new char[byteSize];
    std::string inFile = argv["input"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
    {
        // MakeSure to delete buffer
        if (buffer != nullptr)
            delete[] buffer;

        return RESULT_FAILURE;
    }

    MetaStorageType storage = MetaFileUtil::ConvertToMediaType(volumeType);

    if (_ReadFileInBuffer(inFile, &buffer, byteSize) == RESULT_SUCCESS)
    {
        rc = metaFs->io->Write(fd, byteOffset, byteSize, buffer, storage);
    }
    else
    {
        rc = POS_EVENT_ID::MFS_FILE_READ_FAILED;
    }

    // MakeSure to delete buffer
    if (buffer != nullptr)
        delete[] buffer;

    if (rc != POS_EVENT_ID::SUCCESS)
        return RESULT_FAILURE;

    return RESULT_SUCCESS;
}

int
WbtMetafsCmdHandler::GetFileSize(Args argv)
{
    if (argv.size() < 3)
    {
        std::cout << "mfs_get_file_size Command. Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    int fd = stoi(argv["fd"].get<std::string>());
    std::string arrayName = argv["array"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);
    if (nullptr == metaFs)
        return RESULT_FAILURE;

    size_t fileSize = metaFs->ctrl->GetFileSize(fd, volumeType);
    if (fileSize == 0)
        return RESULT_FAILURE;

    return fileSize;
}

int
WbtMetafsCmdHandler::GetAlignedFileIOSize(Args argv)
{
    if (argv.size() < 3)
    {
        std::cout << "mfs_get_aligned_file_io_size Command. Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    int fd = stoi(argv["fd"].get<std::string>());
    std::string arrayName = argv["array"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);
    if (nullptr == metaFs)
        return RESULT_FAILURE;

    size_t fileSize = metaFs->ctrl->GetAlignedFileIOSize(fd, volumeType);
    if (fileSize == 0)
        return RESULT_FAILURE;

    return fileSize;
}

int
WbtMetafsCmdHandler::DumpInodeInfo(Args argv)
{
    if (argv.size() < 4)
    {
        std::cout << "mfs_dump_inode_info Command. Too few Arguments." << std::endl;
        return RESULT_FAILURE;
    }

    int res = RESULT_SUCCESS;
    MetaFileInodeData metaFileInode;
    std::string parsedJson;
    MetaFileInodeDumpCxt result;
    JsonElement element("");
    std::string metaFile = argv["name"].get<std::string>();
    std::string outJsonFile = argv["output"].get<std::string>();
    std::string arrayName = argv["array"].get<std::string>();
    int type = stoi(argv["volume"].get<std::string>());

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayName);
    if (nullptr == metaFs)
        return RESULT_FAILURE;

    MetaVolumeType volumeType = (MetaVolumeType)type;
    if (type >= (int)MetaVolumeType::Max)
        return RESULT_FAILURE;

    if (false == metaFs->wbt->GetMetaFileInode(metaFile, result, volumeType))
    {
        return RESULT_FAILURE;
    }

    _DumpInodeInfoToJson(&result, element);
    parsedJson = element.ToJson();

    if (_WriteBufferInFile(outJsonFile, parsedJson.c_str(), parsedJson.length()) == RESULT_FAILURE)
    {
        res = RESULT_FAILURE;
    }

    return res;
}

/**
 * Private functions
 */
int
WbtMetafsCmdHandler::_ReadFileInBuffer(std::string fileName, char** buffer, uint32_t& fileSize)
{
    int res = RESULT_SUCCESS;
    std::ifstream inFileStream;

    inFileStream.open(fileName, std::ios::in | std::ios::binary | std::ios::ate); // ate means pointer is at end of file.

    // If fileSize=-1 is passed calculate fileSize from file.
    if (inFileStream.is_open())
    {
        if (fileSize == 0)
        {
            // Makesure file is opend with ios::ate
            fileSize = inFileStream.tellg();
        }

        *buffer = new char[fileSize];
        memset(*buffer, 0, fileSize);
        inFileStream.seekg(0, std::ios::beg);
        inFileStream.read(*buffer, fileSize);
        inFileStream.close();
    }
    else
    {
        std::cout << "[Error] Could not open given file " + fileName << std::endl;
        res = RESULT_FAILURE;
    }

    // MakeSure to delete buffer in caller
    return res;
}

int
WbtMetafsCmdHandler::_WriteBufferInFile(std::string fileName, const char* buffer, int bufferSize)
{
    int res = RESULT_SUCCESS;
    std::ofstream outFileStream;

    outFileStream.open(fileName, std::ios_base::out | std::ios::binary);

    if (outFileStream.is_open())
    {
        outFileStream.seekp(0, std::ios::beg);
        outFileStream.write(buffer, bufferSize);
        outFileStream.close();
    }
    else
    {
        std::cout << "[Error] Could not open given file " + fileName << std::endl;
        res = RESULT_FAILURE;
    }

    return res;
}

void
WbtMetafsCmdHandler::_DumpFilesInfoToJsonElement(MetaFileInfoDumpCxt data, JsonArray& arrayElement)
{
    JsonElement fileData("");

    /*fileData.SetAttribute(JsonAttribute("fileName", quot + data.fileName + quot));
    fileData.SetAttribute(JsonAttribute("fd", quot + std::to_string(static_cast<int>(data.fd)) + quot));
    fileData.SetAttribute(JsonAttribute("size", quot + std::to_string(static_cast<uint32_t>(data.size)) + quot));
    fileData.SetAttribute(JsonAttribute("ctime", quot + std::to_string(data.ctime) + quot));
    fileData.SetAttribute(JsonAttribute("location", quot + data.location + quot));
    */

    fileData.SetAttribute(JsonAttribute("fileName", "\"" + data.fileName + "\""));
    fileData.SetAttribute(JsonAttribute("fd", std::to_string(static_cast<int>(data.fd))));
    fileData.SetAttribute(JsonAttribute("size", std::to_string(static_cast<uint32_t>(data.size))));
    fileData.SetAttribute(JsonAttribute("ctime", std::to_string(data.ctime)));
    fileData.SetAttribute(JsonAttribute("lpnBase", std::to_string(data.lpnBase)));
    fileData.SetAttribute(JsonAttribute("lpnCount", std::to_string(data.lpnCount)));
    fileData.SetAttribute(JsonAttribute("location", "\"" + data.location + "\""));

    arrayElement.AddElement(fileData);
}

void
WbtMetafsCmdHandler::_DumpInodeInfoToJson(MetaFileInodeDumpCxt *data, JsonElement& element)
{
    JsonElement metaInode("metaInodeInfo");
    JsonElement fileProperty("fileProperty");
    JsonElement fileExtentMap("extentMap");

    // Set returnData
    metaInode.SetAttribute(JsonAttribute("inUse", std::to_string(data->inodeInfo.data.field.inUse)));
    metaInode.SetAttribute(JsonAttribute("fd", std::to_string(static_cast<int>(data->inodeInfo.data.field.fd))));
    metaInode.SetAttribute(JsonAttribute("fileName", "\"" + std::string(data->inodeInfo.data.field.fileName) + "\""));
    metaInode.SetAttribute(JsonAttribute("fileByteSize", std::to_string(static_cast<uint32_t>(data->inodeInfo.data.field.fileByteSize))));
    metaInode.SetAttribute(JsonAttribute("dataChunkSize", std::to_string(static_cast<uint32_t>(data->inodeInfo.data.field.dataChunkSize))));
    metaInode.SetAttribute(JsonAttribute("dataLocation", std::to_string(static_cast<int>(data->inodeInfo.data.field.dataLocation))));

    // Set pageMap
    fileExtentMap.SetAttribute(JsonAttribute("baseMetaLpn", std::to_string(static_cast<uint64_t>(data->inodeInfo.data.field.extentMap[0].GetStartLpn()))));
    fileExtentMap.SetAttribute(JsonAttribute("pageCnt", std::to_string(static_cast<uint64_t>(data->inodeInfo.data.field.extentMap[0].GetCount()))));

    // Set propertry
    fileProperty.SetAttribute(JsonAttribute("integrity", std::to_string(static_cast<int>(data->inodeInfo.data.field.fileProperty.integrity))));
    fileProperty.SetAttribute(JsonAttribute("ioAccPattern", std::to_string(static_cast<int>(data->inodeInfo.data.field.fileProperty.ioAccPattern))));
    fileProperty.SetAttribute(JsonAttribute("ioOpType", std::to_string(static_cast<int>(data->inodeInfo.data.field.fileProperty.ioOpType))));

    metaInode.SetElement(fileProperty);
    metaInode.SetElement(fileExtentMap);
    element.SetElement(metaInode);
}

int
WbtMetafsCmdHandler::SetupMetaFioTest(Args argv)
{
    if (MetaIoHandler::index >= 4)
    {
        cout << "Cannot set handlers more than max count" << endl;

        return RESULT_FAILURE;
    }

    std::string target(argv["name"].get<std::string>());
    target.append(to_string(MetaIoHandler::index));
    MetaIoHandler ioHandler(stoi(argv["size"].get<std::string>()));
    unvmf_io_handler handler = {.submit = MetaIoHandler::submitHandlerList[MetaIoHandler::index], .complete = MetaIoHandler::MetaFsIOCompleteHandler};
    spdk_bdev_pos_register_io_handler(target.c_str(), handler);
    MetaIoHandler::index++;

    std::cout << "Testing environment for metafs fio has been setup! [target vol]=" << target.c_str() << " [fd]=" << argv["size"].get<std::string>() << std::endl;

    return RESULT_SUCCESS;
}
} // namespace pos
