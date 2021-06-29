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

#include <string>
#include "metafs_file_control_api_fake.h"
#include "metafs_config.h"
#include "metafs_log.h"
#include "src/logger/logger.h"

namespace pos
{
MetaFsFileControlApi::~MetaFsFileControlApi(void)
{
    for (auto& item : fd2FileNameMap)
    {
        Close(item.first);
    }
}

const char*
MetaFsFileControlApi::_GetFakeMetaFileDir(void)
{
    return FAKE_META_FILE_DIR;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::CreateVolume(std::string& fileName, std::string& arrayName, uint64_t fileByteSize, MetaFilePropertySet prop)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;

    std::string absFilePath;
    absFilePath.append(_GetFakeMetaFileDir());
    absFilePath.append(fileName);
    int fd = creat(absFilePath.c_str(), O_CREAT | O_RDWR);
    if (fd == -1)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_CREATE_FAILED,
            "File creation failed due to unknown error. file={}, rc=",
            fileName.c_str(), fd);
        rc.sc = POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }
    else
    {
        int res = lseek(fd, fileByteSize - 1, SEEK_SET);
        if (res < 0)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_SEEK_FAILED,
                "lseek failed...");
            assert(false);
        }
        res = write(fd, "", 1);
        if (res < 0)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_WRITE_FAILED,
                "write failed...");
            assert(false);
        }
        close(fd);
        rc.sc = POS_EVENT_ID::SUCCESS;

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} has been created successfully.",
            absFilePath.c_str());
    }
    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::Delete(std::string& fileName, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    rc.sc = POS_EVENT_ID::SUCCESS;

    int fd = -1;
    std::string absFilePath;
    absFilePath.append(_GetFakeMetaFileDir());
    absFilePath.append(fileName);

    if ((fd = open(absFilePath.c_str(), O_RDWR)) == -1)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED,
            "File open failed due to an error (e.g.File not found, etc.). rc={}",
            fd);
        rc.sc = POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
    }
    else if (close(fd) == -1)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED,
            "File open failed due to an error (e.g.File not found, etc.). rc={}",
            fd);
        rc.sc = POS_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::Open(std::string& fileName, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;
    rc.sc = POS_EVENT_ID::SUCCESS;

    int fd = -1;
    std::string absFilePath;
    absFilePath.append(_GetFakeMetaFileDir());
    absFilePath.append(fileName);

    if ((fd = open(absFilePath.c_str(), O_RDWR)) == -1)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED,
            "File open failed due to an error (e.g.File not found, etc.). rc={}",
            fd);
        rc.sc = POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
    }
    else
    {
        auto item = fileContentMap.find(absFilePath);
        if (item != fileContentMap.end())
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED,
                "Attempting to open the file more than once.");
            // rc.returnData = fd;
            // fd2FileNameMap.insert(make_pair(fd, absFilePath));
            rc.sc = POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
        }
        else
        {
            SimpleMetaFileContent fileContent;
            fileContent.fileName = absFilePath;
            fileContentMap.insert(make_pair(absFilePath, fileContent));
            fd2FileNameMap.insert(make_pair(fd, absFilePath));
            rc.sc = POS_EVENT_ID::SUCCESS;
            rc.returnData = fd;
            MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "{} has been opened successfully. fd={}",
                absFilePath.c_str(), fd);
        }
    }

    return rc;
}

MetaFsReturnCode<POS_EVENT_ID>
MetaFsFileControlApi::Close(uint32_t fd, std::string& arrayName)
{
    MetaFsReturnCode<POS_EVENT_ID> rc;

    int res = close(fd);
    if (res != 0)
    {
        rc.sc = POS_EVENT_ID::MFS_FILE_CLOSE_FAILED;

        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_CLOSE_FAILED,
            "File close failed due to unknown error. rc={}", res);
    }
    else
    {
        auto item = fd2FileNameMap.find(fd);
        assert(item != fd2FileNameMap.end());

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} has been closed.", item->second.c_str());

        fileContentMap.erase(item->second);
        fd2FileNameMap.erase(fd);
        rc.sc = POS_EVENT_ID::SUCCESS;
    }
    return rc;
}
} // namespace pos
