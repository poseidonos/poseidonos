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

#include "spdk_bdev_caller.h"


extern "C" {
#include "spdk/module/bdev/malloc/pos_bdev_malloc.h"
}


using namespace pos;

struct spdk_bdev*
SpdkBdevCaller::SpdkBdevFirst(void)
{
    return spdk_bdev_first();
}

struct spdk_bdev*
SpdkBdevCaller::SpdkBdevNext(struct spdk_bdev* bdev)
{
    return spdk_bdev_next(bdev);
}

struct spdk_bdev*
SpdkBdevCaller::SpdkBdevGetByName(const char* name)
{
    return spdk_bdev_get_by_name(name);
}

uint32_t
SpdkBdevCaller::SpdkPosMallocBdevGetNuma(struct spdk_bdev* bdev)
{
    return spdk_pos_malloc_bdev_get_numa(bdev);
}

struct spdk_io_channel*
SpdkBdevCaller::SpdkBdevGetIoChannel(struct spdk_bdev_desc* desc)
{
    return spdk_bdev_get_io_channel(desc);
}

void
SpdkBdevCaller::SpdkBdevClose(struct spdk_bdev_desc *desc)
{
    spdk_bdev_close(desc);
}

uint32_t
SpdkBdevCaller::SpdkBdevGetBlockSize(const struct spdk_bdev* bdev)
{
    return spdk_bdev_get_block_size(bdev);
}

uint64_t
SpdkBdevCaller::SpdkBdevGetNumBlocks(const struct spdk_bdev* bdev)
{
    return spdk_bdev_get_num_blocks(bdev);
}

int
SpdkBdevCaller::SpdkBdevOpenExt(const char* bdev_name,
    bool write,
    spdk_bdev_event_cb_t event_cb,
    void* event_ctx,
    struct spdk_bdev_desc** desc)
{
    return spdk_bdev_open_ext(bdev_name, write, event_cb, event_ctx, desc);
}

int
SpdkBdevCaller::SpdkBdevQueueIoWait(struct spdk_bdev *bdev,
    struct spdk_io_channel *ch,
    struct spdk_bdev_io_wait_entry *entry)
{
    return spdk_bdev_queue_io_wait(bdev, ch, entry);
}

int
SpdkBdevCaller::SpdkBdevRead(struct spdk_bdev_desc *desc, struct spdk_io_channel *ch,
           void *buf, uint64_t offset, uint64_t nbytes,
           spdk_bdev_io_completion_cb cb, void *cb_arg)
{
    return spdk_bdev_read(desc, ch, buf, offset, nbytes, cb, cb_arg);
}

int
SpdkBdevCaller::SpdkBdevWrite(struct spdk_bdev_desc *desc, struct spdk_io_channel *ch,
        void *buf, uint64_t offset, uint64_t nbytes,
        spdk_bdev_io_completion_cb cb, void *cb_arg)
{
    return spdk_bdev_write(desc, ch, buf, offset, nbytes, cb, cb_arg);
}

void
SpdkBdevCaller::SpdkBdevFreeIo(struct spdk_bdev_io* bdev_io)
{
    spdk_bdev_free_io(bdev_io);
}
