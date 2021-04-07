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

#include "wbt_metafs_cmd_handler.h"

#define SUCCESS 0
#define FAILURE -1

int
WbtMetafsCmdHandler::DumpFilesList(Args argv)
{
    if (argv.size() < 1)
    {
        std::cout << "mfs_dump_files_list Command. Too Few Arguments.\nRequire fileName\n";
        return FAILURE;
    }

    int res = SUCCESS;
    std::string parsedJson;
    std::string outFile;
    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, std::vector<MetaFileInfoDumpCxt>> rt;

    outFile = argv["output"].get<std::string>();
    rt = metaFsMgr.wbt.GetMetaFileList();

    if (rt.sc == MetaFsStatusCodeWBTSpcf::Fail)
    {
        return FAILURE;
    }

    JsonElement element("");
    JsonArray arrayElement("filesInfoList");

    for (unsigned int i = 0; i < rt.returnData.size(); i++)
    {
        _DumpFilesInfoToJsonElement(rt.returnData[i], arrayElement);
    }

    element.SetArray(arrayElement);
    parsedJson = element.ToJson();
    parsedJson += "\n";

    if (_WriteBufferInFile(outFile, parsedJson.c_str(), parsedJson.length()) == FAILURE)
    {
        return FAILURE;
    }

    return res;
}

int
WbtMetafsCmdHandler::CreateFile(Args argv)
{
    if (argv.size() < 5)
    {
        std::cout << "mfs_create_file Command. Too Few Arguments.\nRequire fileName and fileSize\n";
        return FAILURE;
    }

    bool retVal;
    MetaFilePropertySet fileProperty = MetaFilePropertySet();

    std::string fileName = argv["name"].get<std::string>();
    uint32_t fileSizeBytes = stoi(argv["size"].get<std::string>());
    int integrityType = stoi(argv["integrity"].get<std::string>());
    int ioAccPatternType = stoi(argv["access"].get<std::string>());
    int ioOpType = stoi(argv["operation"].get<std::string>());

    fileProperty.integrity = static_cast<MDFilePropIntegrity>(integrityType);
    fileProperty.ioAccPattern = static_cast<MDFilePropIoAccessPattern>(ioAccPatternType);
    fileProperty.ioOpType = static_cast<MDFilePropIoOpType>(ioOpType);

    retVal = metaFsMgr.mgmt.Create(fileName, fileSizeBytes, fileProperty).IsSuccess();

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::OpenFile(Args argv)
{
    if (argv.size() < 1)
    {
        std::cout << "mfs_open_file Command. Too few Arguments.\nRequire fileDesc.\n";
        return FAILURE;
    }

    int fd;

    std::string fileName = argv["name"].get<std::string>();
    fd = metaFsMgr.mgmt.Open(fileName).returnData;

    if (fd == FAILURE)
        return FAILURE;

    return fd;
}

int
WbtMetafsCmdHandler::CloseFile(Args argv)
{
    if (argv.size() < 1)
    {
        std::cout << "mfs_close_file Command. Too few Arguments.\nRequire fileDesc.\n";
        return FAILURE;
    }

    bool retVal;
    int fd;
    fd = stoi(argv["fd"].get<std::string>());
    retVal = metaFsMgr.mgmt.Close(fd).IsSuccess();

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::CreateFileSystem(void)
{
    bool retVal;

    retVal = metaFsMgr.sys.Create().IsSuccess();

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::MountFileSystem(void)
{
    bool retVal;

    retVal = metaFsMgr.sys.Mount().IsSuccess();

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::UmountFileSystem(void)
{
    bool retVal;

    retVal = metaFsMgr.sys.Unmount().IsSuccess();

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::ReadFile(Args argv)
{
    if (argv.size() < 4)
    {
        std::cout << "mfs_read_file Command.Too few Arguments.\nRequire fileDesc fileOffset dataLengthInBytes path/mfsBufferFile.bin.\n";
        return FAILURE;
    }

    bool retVal = false;
    int fd;
    uint32_t byteOffset;
    uint32_t byteSize;
    char* buffer = nullptr;
    std::string outFile;

    fd = stoi(argv["fd"].get<std::string>());

    byteOffset = stoi(argv["offset"].get<std::string>());

    byteSize = stoi(argv["count"].get<std::string>());

    buffer = new char[byteSize];
    outFile = argv["output"].get<std::string>();

    if (buffer == nullptr)
    {
        std::cout << "[Error] Buffer allocation Failed.\n";
        return FAILURE;
    }

    memset(buffer, 0, byteSize);

    retVal = metaFsMgr.io.Read(fd, byteOffset, byteSize, (void*)buffer).IsSuccess();

    if (retVal == true && _WriteBufferInFile(outFile, buffer, byteSize) == SUCCESS)
    {
        // Nothing to do
    }
    else
    {
        retVal = false;
    }

    if (buffer != nullptr)
        delete[] buffer;

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::WriteFile(Args argv)
{
    if (argv.size() < 4)
    {
        std::cout << "mfs_write_file Command.Too few Arguments.\nRequire fileDesc fileOffset dataLengthInBytes path/mfsBufferFile.bin.\n";
        return FAILURE;
    }

    bool retVal = false;
    int fd;
    uint32_t byteOffset;
    uint32_t byteSize;
    char* buffer = nullptr;
    std::string inFile;

    fd = stoi(argv["fd"].get<std::string>());
    byteOffset = stoi(argv["offset"].get<std::string>());
    byteSize = stoi(argv["count"].get<std::string>());
    buffer = new char[byteSize];
    inFile = argv["input"].get<std::string>();

    if (_ReadFileInBuffer(inFile, &buffer, byteSize) == SUCCESS)
    {
        retVal = metaFsMgr.io.Write(fd, byteOffset, byteSize, buffer).IsSuccess();
    }
    else
    {
        retVal = false;
    }

    // MakeSure to delete buffer
    if (buffer != nullptr)
        delete[] buffer;

    if (retVal == false)
        return FAILURE;

    return SUCCESS;
}

int
WbtMetafsCmdHandler::ReadFileAsync(Args argv)
{
    /*int fd;
    uint32_t byteOffset;
    uint32_t byteSize;
    void *buffer;
    void *callback(int);
    // metaFsMgr.io.ReadAsync(fd, byteOffset, byteSize, buffer, callback);    
    // As async functionality testing is not enabled yet.
    */
    return FAILURE;
}

int
WbtMetafsCmdHandler::WriteFileAsync(Args argv)
{
    /*int fd;
    uint32_t byteOffset;
    uint32_t byteSize;
    void *buffer;
    void *callback(int);
    // metaFsMgr.io.WriteAsync(fd, byteOffset, byteSize, buffer, callback);
    // As async functionality testing is not enabled yet.
    */
    return FAILURE;
}

int
WbtMetafsCmdHandler::GetFileSize(Args argv)
{
    if (argv.size() < 1)
    {
        std::cout << "mfs_get_file_size Command. Too few Arguments\nRequire fileDesc.\n";
        return FAILURE;
    }

    int fd;
    size_t fileSize;

    fd = stoi(argv["fd"].get<std::string>());
    fileSize = metaFsMgr.util.GetFileSize(fd);

    if (fileSize == 0)
        return FAILURE;

    return fileSize;
}

int
WbtMetafsCmdHandler::GetAlignedFileIOSize(Args argv)
{
    if (argv.size() < 1)
    {
        std::cout << "mfs_get_aligned_file_io_size Command. Too few Arguments\nRequire fileDesc.\n";
        return FAILURE;
    }

    int fd;
    size_t fileSize;

    fd = stoi(argv["fd"].get<std::string>());
    fileSize = metaFsMgr.util.GetAlignedFileIOSize(fd);

    if (fileSize == 0)
        return FAILURE;

    return fileSize;
}

int64_t
WbtMetafsCmdHandler::GetMaxFileSize(Args argv)
{
    uint64_t maxFileSize = 0;
    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, FileSizeType> rt;

    rt = metaFsMgr.wbt.GetMaxFileSizeLimit();

    if (rt.sc == MetaFsStatusCodeWBTSpcf::Fail)
    {
        return FAILURE;
    }
    maxFileSize = rt.returnData;

    return maxFileSize;
}

int
WbtMetafsCmdHandler::DumpInodeInfo(Args argv)
{
    int res = SUCCESS;
    std::string metaFile;
    std::string outJsonFile;
    MetaFileInodeData metaFileInode;
    std::string parsedJson;
    JsonElement element("");
    metaFile = argv["name"].get<std::string>();
    outJsonFile = argv["output"].get<std::string>();

    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, MetaFileInodeDumpCxt> rt;
    rt = metaFsMgr.wbt.GetMetaFileInode(metaFile);

    if (rt.sc == MetaFsStatusCodeWBTSpcf::Fail)
    {
        return FAILURE;
    }

    _DumpInodeInfoToJson(&rt.returnData, element);
    parsedJson = element.ToJson();

    if (_WriteBufferInFile(outJsonFile, parsedJson.c_str(), parsedJson.length()) == FAILURE)
    {
        res = FAILURE;
    }

    return res;
}

int
WbtMetafsCmdHandler::SetInodeInfo(Args argv)
{
#if 0
    int res = SUCCESS;
    std::string metaFileName = argv["name"].get<std::string>();
    std::string jsonFileName = argv["input"].get<std::string>();
    char* buffer = NULL;
    uint32_t fileSize = 0;
    MetaFileInodeInfo metaFileInode;

    // Declare inode
    if (_ReadFileInBuffer(jsonFileName, &buffer, fileSize) == SUCCESS)
    {
        for (uint32_t i = 0; i < fileSize; i++)
        {
            std::cout << buffer[i];
        }
        buffer[fileSize] = '\0';
        rapidjson::Document jsonDoc;
        jsonDoc.Parse<0>(buffer);

        if (jsonDoc.HasParseError())
        {
            std::cout << "[Error] JSON parse error\n";
            std::cout << jsonDoc.GetParseError() << "\nError Offset ";
            std::cout << jsonDoc.GetErrorOffset() << "\n";
            delete[] buffer;
            return FAILURE;
        }
        if (jsonDoc["metaInodeInfo"].IsObject())
        {
            _SetValuesInMetaFileInode(metaFileInode, jsonDoc["metaInodeInfo"]);
        }
    }
    else
    {
        res = FAILURE;
    }

    if (buffer != NULL)
        delete[] buffer;

    return res;
#endif
    return FAILURE;     // not support yet    
}

int
WbtMetafsCmdHandler::GetFileChecksum(Args argv)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::GetCurrentSystemState(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::GetNextSystemState(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::CorruptFileSystemMBR(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::CorruptFileSystemSignature(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::SetAllBitsInFDInUse(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::GetAllBitsInFDInUse(Args argv)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::GetTotalFreeInodes(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::SetAllInodeEntryInUseBitmap(void)
{
    return FAILURE;
}

int
WbtMetafsCmdHandler::GetAllInodeEntryInUseBitmap(Args argv)
{
    return FAILURE;
}

/**
 * Private functions
 */
int
WbtMetafsCmdHandler::_ReadFileInBuffer(std::string fileName, char** buffer, uint32_t& fileSize)
{
    int res = SUCCESS;
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
        std::cout << "[Error] Could not open given file " + fileName + "\n";
        res = FAILURE;
    }

    // MakeSure to delete buffer in caller
    return res;
}

int
WbtMetafsCmdHandler::_WriteBufferInFile(std::string fileName, const char* buffer, int bufferSize)
{
    int res = SUCCESS;
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
        std::cout << "[Error] Could not open given file " + fileName + "\n";
        res = FAILURE;
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
    fileExtentMap.SetAttribute(JsonAttribute("baseMetaLpn", std::to_string(static_cast<uint64_t>(data->inodeInfo.data.field.extentMap.baseMetaLpn))));
    fileExtentMap.SetAttribute(JsonAttribute("pageCnt", std::to_string(static_cast<uint64_t>(data->inodeInfo.data.field.extentMap.pageCnt))));

    // Set propertry
    fileProperty.SetAttribute(JsonAttribute("integrity", std::to_string(static_cast<int>(data->inodeInfo.data.field.fileProperty.integrity))));
    fileProperty.SetAttribute(JsonAttribute("ioAccPattern", std::to_string(static_cast<int>(data->inodeInfo.data.field.fileProperty.ioAccPattern))));
    fileProperty.SetAttribute(JsonAttribute("ioOpType", std::to_string(static_cast<int>(data->inodeInfo.data.field.fileProperty.ioOpType))));

    metaInode.SetElement(fileProperty);
    metaInode.SetElement(fileExtentMap);
    element.SetElement(metaInode);
}

void
WbtMetafsCmdHandler::_SetValuesInMetaFileInode(MetaFileInodeInfo& metaFileInode, rapidjson::Value& inodeData)
{
    rapidjson::Value& fileProperty = inodeData["fileProperty"];
    rapidjson::Value& extentMap = inodeData["extentMap"];

    std::cout << "======================================\n";
    std::cout << "inUse : " << inodeData["inUse"].GetInt() << "\n";
    std::cout << "fd " << inodeData["fd"].GetInt() << "\n";
    std::cout << "fileName : " << inodeData["fileName"].GetString() << "\n";
    std::cout << "fileByteSize : " << inodeData["fileByteSize"].GetUint() << "\n";
    std::cout << "dataChunkSize : " << inodeData["dataChunkSize"].GetUint() << "\n";
    std::cout << "baseMetaLpn : " << extentMap["baseMetaLpn"].GetUint64() << "\n";
    std::cout << "pageCnt : " << extentMap["pageCnt"].GetUint64() << "\n";
    std::cout << "integrity : " << fileProperty["integrity"].GetInt() << "\n";
    std::cout << "ioAccPattern " << fileProperty["ioAccPattern"].GetInt() << "\n";
    std::cout << "ioOpType : " << fileProperty["ioOpType"].GetInt() << "\n";
    std::cout << "======================================\n";

    metaFileInode.data.field.inUse = static_cast<bool>(inodeData["inUse"].GetInt());
    metaFileInode.data.field.fd = static_cast<FileFDType>(inodeData["fd"].GetInt());
    metaFileInode.data.field.fileByteSize = static_cast<FileSizeType>(inodeData["fileByteSize"].GetUint());
    metaFileInode.data.field.dataChunkSize = static_cast<FileSizeType>(inodeData["dataChunkSize"].GetUint());
    metaFileInode.data.field.dataLocation = static_cast<MetaStorageType>(inodeData["dataLocation"].GetInt());

    metaFileInode.data.field.extentMap.baseMetaLpn = static_cast<MetaLpnType>(extentMap["baseMetaLpn"].GetUint64());
    metaFileInode.data.field.extentMap.pageCnt = static_cast<MetaLpnType>(extentMap["pageCnt"].GetUint64());

    metaFileInode.data.field.fileProperty.integrity = static_cast<MDFilePropIntegrity>(fileProperty["integrity"].GetInt());
    metaFileInode.data.field.fileProperty.ioAccPattern = static_cast<MDFilePropIoAccessPattern>(fileProperty["ioAccPattern"].GetInt());
    metaFileInode.data.field.fileProperty.ioOpType = static_cast<MDFilePropIoOpType>(fileProperty["ioOpType"].GetInt());
}

#include "src/device/spdk/spdk.hpp"
#include "src/metafs/tool/fio/meta_scheduler.h"
#include "src/scheduler/scheduler_api.h"

int
WbtMetafsCmdHandler::SetupMetaFioTest(Args argv)
{
    if (MetaIoHandler::index >= 4)
    {
        cout << "Cannot set handlers more than max count" << endl;

        return FAILURE;
    }

    std::string target(argv["name"].get<std::string>());
    target.append(to_string(MetaIoHandler::index));
    MetaIoHandler ioHandler(stoi(argv["size"].get<std::string>()));
    unvmf_io_handler handler = {.submit = MetaIoHandler::submitHandlerList[MetaIoHandler::index], .complete = MetaIoHandler::MetaFsIOCompleteHandler};
    spdk_bdev_ibof_register_io_handler(target.c_str(), handler, NULL);
    MetaIoHandler::index++;

    std::cout << "Testing environment for metafs fio has been setup! [target vol]=" << target.c_str() << " [fd]=" << argv["size"].get<std::string>() << std::endl;

    return SUCCESS;
}

/*
int WbtMetafsCmdHandler::GetFileDesc(std::string fileName)
{
    // return metaFsMgr.util.GetFileDesc(fileName);
}
*/
