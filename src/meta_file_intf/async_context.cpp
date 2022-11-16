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

#include "async_context.h"

#include <boost/format.hpp>
#include <sstream>

namespace pos
{
AsyncMetaFileIoCtx::AsyncMetaFileIoCtx(void)
: error(0),
  opcode(MetaFsIoOpcode::Write),
  fd(-1),
  fileOffset(0),
  length(0),
  buffer(nullptr),
  callback(nullptr),
  ioDoneCheckCallback(nullptr),
  fileInfoUpdated(false),
  ioInfoUpdated(false),
  callbackUpdated(false)
{
}

void
AsyncMetaFileIoCtx::HandleIoComplete(void* data)
{
    if (ioDoneCheckCallback)
        error = ioDoneCheckCallback(data);
    if (callback)
        callback(this);
}

char*
AsyncMetaFileIoCtx::GetBuffer(void)
{
    return buffer;
}

MetaIoCbPtr
AsyncMetaFileIoCtx::GetCallback(void)
{
    return callback;
}

int
AsyncMetaFileIoCtx::GetError(void) const
{
    return error;
}

uint64_t
AsyncMetaFileIoCtx::GetLength(void) const
{
    return length;
}

MetaFsIoOpcode
AsyncMetaFileIoCtx::GetOpcode(void) const
{
    return opcode;
}

int
AsyncMetaFileIoCtx::GetFd(void) const
{
    return fd;
}

uint64_t
AsyncMetaFileIoCtx::GetFileOffset(void) const
{
    return fileOffset;
}

std::string
AsyncMetaFileIoCtx::ToString(void) const
{
    std::ostringstream oss;
    oss << "opcode:" << (int)opcode << ", ";
    oss << "fd:" << fd << ", ";
    oss << "fileOffset:" << (int)fileOffset << ", ";
    oss << "length:" << (int)length << ", ";
    oss << "buffer:" << ((buffer == nullptr) ? "nullptr" : "0x" + (boost::format("%x") % (uint64_t)(uint64_t*)buffer).str()) << ", ";
    oss << "callback:" << ((callback == nullptr) ? "nullptr" : "not nullptr") << ", ";
    oss << "error:" << error << ", ";
    oss << "ioDoneCheckCallback:" << ((ioDoneCheckCallback == nullptr) ? "nullptr" : "not nullptr") << ", ";

    return oss.str();
}

void
AsyncMetaFileIoCtx::SetFileInfo(int fd_, MetaFileIoCbPtr ioDoneCallback_)
{
    this->fd = fd_;
    this->ioDoneCheckCallback = ioDoneCallback_;

    fileInfoUpdated = true;
}

void
AsyncMetaFileIoCtx::SetIoInfo(MetaFsIoOpcode opcode_, uint64_t fileOffset_,
    uint64_t length_, char* buffer_)
{
    this->opcode = opcode_;
    this->fileOffset = fileOffset_;
    this->length = length_;
    this->buffer = buffer_;

    ioInfoUpdated = true;
}

void
AsyncMetaFileIoCtx::SetCallback(MetaIoCbPtr callback_)
{
    this->callback = callback_;

    callbackUpdated = true;
}

bool
AsyncMetaFileIoCtx::IsReadyToUse(void) const
{
    return (fileInfoUpdated == true && ioInfoUpdated == true && callbackUpdated == true);
}

} // namespace pos
