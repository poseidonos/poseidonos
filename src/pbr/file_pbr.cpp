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

#include "file_pbr.h"
#include "revision.h"
#include "src/pbr/load/pbr_file_loader.h"
#include "src/pbr/load/pbr_selector.h"
#include "src/pbr/update/pbr_file_updater.h"
#include "src/helper/file/file.h"
#include "src/helper/file/directory.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pbr
{
const string FilePbr::dirPath = "/etc/pos/pbr/";

int
FilePbr::Load(vector<AteData*>& ateListOut)
{
    vector<string> files;
    GetFilesInTheDirectory(dirPath, files);
    vector<string> fileList;
    int ret = 0;
    for (string file : files)
    {
        string ext = GetFileExtension(file);
        string filePath = _GetFullPath(file);
        if (ext == "pbr")
        {
            fileList.push_back(filePath);
        }
    }
    if (fileList.size() == 0)
    {
        ret = EID(PBR_LOAD_NO_VALID_PBR_FOUND);
        POS_TRACE_INFO(ret, "");
        return ret;
    }

    unique_ptr<IPbrLoader> loader = make_unique<PbrFileLoader>(fileList);
    ret = loader->Load(ateListOut);
    if (ret == 0)
    {
        ret = PbrSelector::Select(ateListOut);
    }
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(PBR_LOAD_SUCCESS), "");
    }
    else
    {
        POS_TRACE_WARN(ret, "");
    }
    return ret;
}

int
FilePbr::Reset(void)
{
    int ret = 0;
    vector<string> files;
    GetFilesInTheDirectory(dirPath, files);
    for (string file : files)
    {
        string ext = GetFileExtension(file);
        if (ext == "pbr")
        {
            string filePath = _GetFullPath(file);
            unique_ptr<IPbrUpdater> updater = make_unique<PbrFileUpdater>(REVISION, filePath);
            int result = updater->Clear();
            if (result == 0)
            {
                POS_TRACE_DEBUG(EID(PBR_RESET_SUCCESS), "file_name:{}", file);
            }
            else
            {
                ret = result;
                POS_TRACE_WARN(result, "file_name:{}", file);
            }
        }
    }
    return ret;
}

int
FilePbr::Reset(string arrayName)
{
    string filePath = _GetFullPath(arrayName + ".pbr");
    unique_ptr<IPbrUpdater> updater = make_unique<PbrFileUpdater>(REVISION, filePath);
    int ret = updater->Clear();
    if (ret == 0)
    {
        POS_TRACE_DEBUG(EID(PBR_RESET_SUCCESS), "array_name:{}", arrayName);
    }
    else
    {
        POS_TRACE_WARN(ret, "{}", arrayName);
    }
    return ret;
}

int
FilePbr::Update(AteData* ateData)
{
    if (DirExists(dirPath) == false)
    {
        MakeDir(dirPath);
    }

    string filePath = _GetFullPath(ateData->arrayName + ".pbr");
    unique_ptr<IPbrUpdater> updater = make_unique<PbrFileUpdater>(REVISION, filePath);
    int ret = updater->Update(ateData);
    return ret;
}

string
FilePbr::_GetFullPath(string fileName)
{
    return dirPath + fileName;
}
} // namespace pbr
