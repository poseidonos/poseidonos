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

#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include <mutex>
#include "metafs_aiocb_cxt.h"
#include "src/metafs/metafs.h"
#include "spdk/pos_volume.h"

namespace pos
{
extern const char* META_FIO_TARGET_FILE;

class MetaIoHandler
{
public:
    explicit MetaIoHandler(int fd)
    {
        if (MetaIoHandler::index >= 4)
            return;

        if (submitHandlerList[0] == nullptr)
        {
            submitHandlerList[0] = &MetaIoHandler::IoSubmitHandler0;
            submitHandlerList[1] = &MetaIoHandler::IoSubmitHandler1;
            submitHandlerList[2] = &MetaIoHandler::IoSubmitHandler2;
            submitHandlerList[3] = &MetaIoHandler::IoSubmitHandler3;
        }

        MetaIoHandler::fdList[MetaIoHandler::index] = fd;
    }

    static int IoSubmitHandler0(struct pos_io* io);
    static int IoSubmitHandler1(struct pos_io* io);
    static int IoSubmitHandler2(struct pos_io* io);
    static int IoSubmitHandler3(struct pos_io* io);
    static void MetaFsIOCompleteHandler(void);

    static int MetaFsIOSubmitHandler(struct pos_io* io, int fd);

    static unvmf_submit_handler submitHandlerList[4];
    static int fdList[4];
    static int index;

    // only for test
    static MetaFs* metaFs;
};

class MetaIOScheduler
{
public:
    void
    SetMetaFIOTargetFD(int fd)
    {
        targetMetaFioFD = fd;
    }
    int
    GetMetaFIOTargetFD(void)
    {
        return targetMetaFioFD;
    }
    static void HandleMetaIoCompletion(void* arg1);
    void HandleIOCallback(void* data);

private:
    int targetMetaFioFD = INT_MAX;
};

extern MetaIOScheduler metaioScheduler;

class MetaFioAIOCxt : public MetaFsAioCbCxt
{
public:
    explicit MetaFioAIOCxt(MetaFsIoOpcode opcode, uint32_t fd, int arrayId, size_t soffset, size_t nbytes, void* buf, MetaFsAioCallbackPointer func, pos_io* io, uint32_t reactor)
    : MetaFsAioCbCxt(opcode, fd, arrayId, soffset, nbytes, buf, func),
      io(io),
      reactor(reactor)
    {
    }

    pos_io*
    GetIBoFIOCxt(void)
    {
        return io;
    }

    uint32_t
    GetReactor(void)
    {
        return reactor;
    }

private:
    pos_io* io;
    uint32_t reactor;
};
} // namespace pos
