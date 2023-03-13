/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "src/metafs/mvm/volume/active_file_list.h"

namespace pos
{
ActiveFileList::ActiveFileList(void)
{
}

ActiveFileList::~ActiveFileList(void)
{
}

bool
ActiveFileList::CheckFileInActive(const FileDescriptorType fd) const
{
    if (activeFiles_.find(fd) == activeFiles_.end())
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "File descriptor {} is not found in active fd list", fd);
        return false;
    }
    return true;
}

POS_EVENT_ID
ActiveFileList::AddFileInActiveList(const FileDescriptorType fd)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    if (activeFiles_.find(fd) != activeFiles_.end())
    {
        rc = EID(MFS_FILE_OPEN_REPETITIONARY);
        POS_TRACE_ERROR((int)rc,
            "You attempt to open fd {} file twice. It is not allowed", fd);
    }
    else
    {
        activeFiles_.insert(fd);
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "File descriptor {} is added in active fd list", fd);
    }

    return rc;
}

void
ActiveFileList::RemoveFileFromActiveList(const FileDescriptorType fd)
{
    if (activeFiles_.find(fd) == activeFiles_.end())
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "File descriptor {} is not active file.", fd);
        assert(false);
    }

    activeFiles_.erase(fd);
    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "File descriptor {} is removed from active fd list, remained active file count: {}",
        fd, activeFiles_.size());
}

size_t
ActiveFileList::GetFileCountInActive(void) const
{
    return activeFiles_.size();
}
} // namespace pos
