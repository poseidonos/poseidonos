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

#include "fake_pbr.h"
#include "content_serializer.h"
#include "src/helper/file/file.h"
#include "src/helper/file/directory.h"
#include "src/logger/logger.h"


#include <fstream>
#include <memory.h>

namespace pbr
{
const string FakePbr::dirPath = "/etc/pos/pbr/";

int
FakePbr::Load(vector<AteData*>& out)
{
    vector<string> files;
    int ret = GetFilesInTheDirectory(dirPath, files);
    if (ret == 0)
    {
        for (string file : files)
        {
            string ext = GetFileExtension(file);
            string filePath = GetFullPath(file);
            if (ext == "pbr")
            {
                uint32_t length = ContentSerializer::GetContentSize();
                uint64_t startOffset = ContentSerializer::GetContentStartLba();
                char* rawData = new char[length];
                ret = Read(filePath, rawData, startOffset, length);
                if (ret == 0)
                {
                    POS_TRACE_TRACE(EID(PBR_DEBUG_MSG), "read success:{}", filePath);
                    AteData* ateData = new AteData();
                    ret = ContentSerializer::Deserialize(ateData, rawData);
                    if (ret == 0)
                    {
                        POS_TRACE_DEBUG(EID(PBR_LOAD_SUCCESS), "file_name:{}", filePath);
                        out.push_back(ateData);
                    }
                }
                else
                {
                    POS_TRACE_WARN(ret, "file_name:{}", filePath);
                }
                delete rawData;
            }
        }
    }
    return ret;
}

int
FakePbr::Reset()
{
    vector<string> files;
    int ret = GetFilesInTheDirectory(dirPath, files);
    if (files.size() == 0)
    {
        POS_TRACE_DEBUG(EID(PBR_RESET), "no files exist");
        return 0;
    }
    if (ret == 0)
    {
        for (string file : files)
        {
            string ext = GetFileExtension(file);
            if (ext == "pbr")
            {
                string filePath = GetFullPath(file);
                remove(filePath.c_str());
                POS_TRACE_DEBUG(EID(PBR_RESET_SUCCESS), "{}", filePath);
            }
        }
    }
    return ret;
}

int
FakePbr::Reset(string arrayName)
{
    string filePath = GetFullPath(arrayName + ".pbr");
    remove(filePath.c_str());
    POS_TRACE_DEBUG(EID(PBR_RESET_SUCCESS), "{}", filePath);
    return 0;
}

int
FakePbr::Update(AteData* ateData)
{
    if (DirExists(dirPath) == false)
    {
        MakeDir(dirPath);
    }

    string filePath = GetFullPath(ateData->arrayName + ".pbr");
    uint32_t length = ContentSerializer::GetContentSize();
    char* rawData = new char[length];
    int ret = ContentSerializer::Serialize(rawData, ateData);
    if (ret == 0)
    {
        uint64_t startOffset = ContentSerializer::GetContentStartLba();
        Write(filePath, rawData, startOffset, length);
        POS_TRACE_DEBUG(EID(PBR_UPDATE_SUCCESS), "{}", filePath);
    }
    delete rawData;
    return ret;
}

int
FakePbr::Read(string filePath, char* dataOut, uint64_t startOffset, uint32_t length)
{
    POS_TRACE_TRACE(EID(PBR_DEBUG_MSG), "read from:{}", filePath);
    std::ifstream checkFileExists(filePath);
    if(!checkFileExists.is_open())
    {
        POS_TRACE_TRACE(EID(PBR_DEBUG_MSG), "read ERROR:{}", filePath);
        return -1;
    }

    ifstream f(filePath, ios::in | ios::ate);
    f.seekg(startOffset, ios::beg);
    f.read(dataOut, length);
    f.close();
    return 0;
}

int
FakePbr::Write(string filePath, char* data, uint64_t startOffset, uint32_t length)
{
    fstream file;
    file.open(filePath, ios_base::out | ios_base::in);
    if(file.is_open() == false)
    {
        // file does not exist so create a file
        file.open(filePath, ios_base::out);
        file.close();
    }

    ofstream f(filePath, ios::in | ios::ate);
    f.seekp(startOffset, ios::beg);
    f.write(data, length);
    f.close();
    return 0;
}

string
FakePbr::GetFullPath(string fileName)
{
    return dirPath + fileName;
}
} // namespace pbr
