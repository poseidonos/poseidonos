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
#include "meta_scheduler.h"
#include "src/metafs/include/metafs_service.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/event_scheduler/spdk_event_scheduler.h"

extern "C"
{
#include "spdk/bdev_module.h"
#include "spdk/pos_volume.h"
}

namespace pos
{
MetaIOScheduler metaioScheduler;
const char* META_FIO_TARGET_FILE = "bdev";
uint32_t g_meta_outstandingCmd = 0;

void
MetaIOScheduler::HandleMetaIoCompletion(void* arg1)
{
    pos_io* io = static_cast<pos_io*>(arg1);
    if (io->complete_cb)
    {
        io->complete_cb(io, POS_IO_STATUS_SUCCESS); // always success for perf test
    }
}

void
MetaIOScheduler::HandleIOCallback(void* data)
{
    MetaFioAIOCxt* cxt = reinterpret_cast<MetaFioAIOCxt*>(data);
    pos::EventFrameworkApiSingleton::Instance()->SendSpdkEvent(
        cxt->GetReactor(),
        HandleMetaIoCompletion, cxt->GetIBoFIOCxt());

    delete cxt;
    g_meta_outstandingCmd--;
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE), "callback done");
}

#include "meta_io_manager.h"

unvmf_submit_handler MetaIoHandler::submitHandlerList[4] = {
    nullptr,
};
int MetaIoHandler::fdList[4] = {
    0,
};
int MetaIoHandler::index = 0;
MetaFs* MetaIoHandler::metaFs = nullptr;

int
MetaIoHandler::IoSubmitHandler0(struct pos_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[0]);
}

int
MetaIoHandler::IoSubmitHandler1(struct pos_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[1]);
}

int
MetaIoHandler::IoSubmitHandler2(struct pos_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[2]);
}

int
MetaIoHandler::IoSubmitHandler3(struct pos_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[3]);
}

int
MetaIoHandler::MetaFsIOSubmitHandler(struct pos_io* io, int fd)
{
    assert(io->ioType == IO_TYPE::READ || io->ioType == IO_TYPE::WRITE);
    // assert(io->length == 4096); // LIMITATION: currently 4KB workload is targeted to metafs perf evaluation!

    POS_EVENT_ID rc_io;
    assert(fd != INT_MAX);

    MetaFsIoOpcode opcode = (io->ioType == IO_TYPE::READ) ? MetaFsIoOpcode::Read : MetaFsIoOpcode::Write;
    uint32_t alignedIOSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    uint64_t soffset = ((io->offset / alignedIOSize) * alignedIOSize);
    uint32_t reactor = pos::EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    int arrayId = io->array_id;
    MetaFsAioCbCxt* aiocb = new MetaFioAIOCxt(opcode, fd, arrayId, soffset, alignedIOSize, io->iov->iov_base,
        AsEntryPointParam1(&MetaIOScheduler::HandleIOCallback, &metaioScheduler),
        io, reactor);

    // only for test
    if (nullptr != metaFs)
    {
        rc_io = metaFs->io->SubmitIO(aiocb);

        if (nullptr != aiocb)
            delete aiocb;
    }
    else
    {
        rc_io = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayId)->io->SubmitIO(aiocb);
    }
    g_meta_outstandingCmd++;

    return (EID(SUCCESS) == rc_io) ? 0 : 1;
}

void
MetaIoHandler::MetaFsIOCompleteHandler(void)
{
    // nothing
}
} // namespace pos
