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

#include "meta_scheduler.h"

#include "mfs.h"
#include "src/device/event_framework_api.h"

MetaIOScheduler metaioScheduler;
const char* META_FIO_TARGET_FILE = "bdev";
uint32_t g_meta_outstandingCmd = 0;

extern "C"
{
#include "spdk/bdev_module.h"
#include "spdk/ibof_volume.h"
}

void HandleMetaIoCompletion(void* arg1, void* arg2);

void
HandleMetaIoCompletion(void* arg1, void* arg2)
{
    ibof_io* io = static_cast<ibof_io*>(arg1);
    if (io->complete_cb)
    {
        io->complete_cb(io, IBOF_IO_STATUS_SUCCESS); // always success for perf test
    }
}

void
MetaIOScheduler::HandleIOCallback(void* data)
{
    MetaFioAIOCxt* cxt = reinterpret_cast<MetaFioAIOCxt*>(data);
    ibofos::EventFrameworkApi::SendSpdkEvent(
        cxt->GetReactor(),
        HandleMetaIoCompletion, cxt->GetIBoFIOCxt(), nullptr);

    delete cxt;
    g_meta_outstandingCmd--;
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE, "callback done");
}

#include "meta_io_manager.h"

unvmf_submit_handler MetaIoHandler::submitHandlerList[4] = {
    nullptr,
};
int MetaIoHandler::fdList[4] = {
    0,
};
int MetaIoHandler::index = 0;

int
MetaIoHandler::IoSubmitHandler0(struct ibof_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[0]);
}

int
MetaIoHandler::IoSubmitHandler1(struct ibof_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[1]);
}

int
MetaIoHandler::IoSubmitHandler2(struct ibof_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[2]);
}

int
MetaIoHandler::IoSubmitHandler3(struct ibof_io* io)
{
    return MetaIoHandler::MetaFsIOSubmitHandler(io, MetaIoHandler::fdList[3]);
}

int
MetaIoHandler::MetaFsIOSubmitHandler(struct ibof_io* io, int fd)
{
    assert(io->ioType == IO_TYPE::READ || io->ioType == IO_TYPE::WRITE);
    // assert(io->length == 4096); // LIMITATION: currently 4KB workload is targeted to metafs perf evaluation!

    MetaFsReturnCode<IBOF_EVENT_ID> rc_io;
    assert(fd != INT_MAX);
#if 1
    // NOTE: In order to issue the command performing single 4KB internal operation, it has to aligned specific data chunk size of metafs.
    //       Typical data chunk size of each meta page is 4032Byte which is the value excluded meta control info in meta page.
    //       In real user scenario, consumer of metafs issues io awaring this aligned data chunk size. So the assumption of below is not unrealistic
    MetaFsIoOpcode opcode = (io->ioType == IO_TYPE::READ) ? MetaFsIoOpcode::Read : MetaFsIoOpcode::Write;
    uint32_t alignedIOSize = metaFsMgr.util.GetAlignedFileIOSize(fd);
    assert (alignedIOSize != 0);

    uint64_t soffset = ((io->offset / alignedIOSize) * alignedIOSize);
    uint64_t nbytes = alignedIOSize;
    uint32_t reactor = ibofos::EventFrameworkApi::GetCurrentReactor();
    MetaFsAioCbCxt* aiocb = new MetaFioAIOCxt(opcode, fd, soffset, nbytes, io->iov->iov_base,
        AsEntryPointParam1(&MetaIOScheduler::HandleIOCallback, &metaioScheduler),
        io, reactor);
    rc_io = metaFsMgr.io.SubmitIO(aiocb);
    g_meta_outstandingCmd++;

#else // testing for sync io
    switch (io->ioType)
    {
        case IO_TYPE::READ:
        {
            rc_io = metaFsMgr.io.Read(fd, (uint64_t)io->offset, (uint64_t)io->length, io->iov->iov_base);
        }
        break;
        case IO_TYPE::WRITE:
        {
            rc_io = metaFsMgr.io.Write(fd, (uint64_t)io->offset, (uint64_t)io->length, io->iov->iov_base);
        }
        break;
    }
    io->complete_cb(io, 0);
#endif

    return rc_io.IsSuccess() ? 0 : 1;
}

void
MetaIoHandler::MetaFsIOCompleteHandler(void)
{
    // nothing
}
