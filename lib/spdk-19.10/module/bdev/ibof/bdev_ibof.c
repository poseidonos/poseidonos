/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Samsung Corporation.
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
 *     * Neither the name of Samsung Corporation nor the names of its
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

#include "spdk/stdinc.h"

#include "bdev_ibof.h"
#include "spdk/bdev.h"
#include "spdk/conf.h"
#include "spdk/endian.h"
#include "spdk/env.h"
#include "spdk/copy_engine.h"
#include "spdk/json.h"
#include "spdk/thread.h"
#include "spdk/queue.h"
#include "spdk/string.h"
#include "spdk/likely.h"

#include "spdk/bdev_module.h"
#include "spdk_internal/log.h"
#include "spdk/ibof.h"
#include "spdk/ibof_volume.h"
#include "spdk/ibof_nvmf.h"
#include "spdk/event.h"
#include "spdk/ioat.h"

#include "Air_c.h"

struct ibof_disk {
	struct spdk_bdev		disk;
	struct ibof_volume_info		volume;
	void				*ibof_buf;
	TAILQ_ENTRY(ibof_disk)	link;
};

struct ibof_task {
	int				num_outstanding;
	enum spdk_bdev_io_status	status;
};

struct ioat_io_channel {
	struct spdk_ioat_chan	*ioat_ch;
	struct ioat_device	*ioat_dev;
	struct spdk_poller	*poller;
};

struct ibof_io_channel {
};

__thread __typeof__(struct spdk_poller*) per_lcore_ibof_poller;

__thread __typeof__(unsigned int) per_lcore_poller_ref_count;

static struct ibof_task *
__ibof_task_from_copy_task(struct spdk_copy_task *ct)
{
	return (struct ibof_task *)((uintptr_t)ct - sizeof(struct ibof_task));
}

static struct spdk_copy_task *
__copy_task_from_ibof_task(struct ibof_task *mt)
{
	return (struct spdk_copy_task *)((uintptr_t)mt + sizeof(struct ibof_task));
}

static void
ibof_done(void *ref, int status)
{
	struct ibof_task *task = __ibof_task_from_copy_task(ref);

	if (status != 0) {
		if (status == -ENOMEM) {
			task->status = SPDK_BDEV_IO_STATUS_NOMEM;
		} else {
			task->status = SPDK_BDEV_IO_STATUS_FAILED;
		}
	}

	if (--task->num_outstanding == 0) {
		spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task), task->status);
	}
}

static void
ibof_dma_done(void *ref)
{
	ibof_done(ref, 0);
}

static TAILQ_HEAD(, ibof_disk) g_ibof_disks = TAILQ_HEAD_INITIALIZER(g_ibof_disks);

static uint32_t ibof_disk_count = 0;

static int bdev_ibof_initialize(void);
static void bdev_ibof_get_spdk_running_config(FILE *fp);

static int
bdev_ibof_get_ctx_size(void)
{
	return sizeof(struct ibof_task) + spdk_copy_task_size();
}

static struct spdk_bdev_module ibof_if = {
	.name = "iBoF",
	.module_init = bdev_ibof_initialize,
	.config_text = bdev_ibof_get_spdk_running_config,
	.get_ctx_size = bdev_ibof_get_ctx_size,

};

SPDK_BDEV_MODULE_REGISTER(ibof, &ibof_if)

static bool
bdev_ibof_io_type_supported(void *ctx, enum spdk_bdev_io_type io_type)
{
	switch (io_type) {
	case SPDK_BDEV_IO_TYPE_READ:
	case SPDK_BDEV_IO_TYPE_WRITE:
	case SPDK_BDEV_IO_TYPE_FLUSH:
		return true;
	/*
	case SPDK_BDEV_IO_TYPE_RESET:
	case SPDK_BDEV_IO_TYPE_UNMAP:
	case SPDK_BDEV_IO_TYPE_WRITE_ZEROES:
		return true;
	*/

	default:
		return false;
	}
}


static void
bdev_ibof_get_spdk_running_config(FILE *fp)
{
	int num_ibof_volume = 0;
	uint64_t ibof_buffer_size = 0;
	struct ibof_disk *mdisk;

	/* count number of ibof volumes, get volume buffer size */
	TAILQ_FOREACH(mdisk, &g_ibof_disks, link) {
		if (0 == ibof_buffer_size) {
			/* assume all ibof luns the same size */
			ibof_buffer_size = (mdisk->disk.blocklen * mdisk->disk.blockcnt) / (1024*1024);
		}
		num_ibof_volume++;
	}

	if (num_ibof_volume > 0) {
		fprintf(fp,
			"\n"
			"# Users may change this section to create a different number or volume size of\n"
			"# ibof volume.\n"
			"# This will generate %d Volumes with a ibof-allocated backend. Each Volume\n"
			"# is mapped with iBOF volume 1:1 and \n"
			"# will has %" PRIu64 "MB in size buffer and these will be named Volume0 through Volume%d.\n"
			"# Not all Volumes defined here are necessarily used below.\n"
			"[Volume]\n"
			"  NumberOfVolume %d\n"
			"  VolumeBufferInMB %" PRIu64 "\n",
			num_ibof_volume, ibof_buffer_size,
			num_ibof_volume - 1, num_ibof_volume,
			ibof_buffer_size);
	}
}


static void
ibof_disk_free(struct ibof_disk *ibof_disk)
{
	if (!ibof_disk) {
		return;
	}

	free(ibof_disk->disk.name);
	spdk_dma_free(ibof_disk->ibof_buf);
	spdk_dma_free(ibof_disk);
}

static int
bdev_ibof_destruct(void *ctx)
{
	struct ibof_disk *ibof_disk = ctx;

	TAILQ_REMOVE(&g_ibof_disks, ibof_disk, link);
	ibof_disk_free(ibof_disk);
	return 0;
}

static int
bdev_ibof_check_iov_len(struct iovec *iovs, int iovcnt, size_t nbytes)
{
	int i;

	for (i = 0; i < iovcnt; i++) {
		if (nbytes < iovs[i].iov_len) {
			return 0;
		}

		nbytes -= iovs[i].iov_len;
	}

	return nbytes != 0;
}

static void
bdev_ibof_ramdisk_readv(struct ibof_disk *mdisk, struct spdk_io_channel *ch,
		  struct ibof_task *task,
		  struct iovec *iov, int iovcnt, size_t len, uint64_t offset)
{
	int64_t res = 0;
	void *src = mdisk->ibof_buf + offset;
	int i;

	if (bdev_ibof_check_iov_len(iov, iovcnt, len)) {
		spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task),
				      SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}

	SPDK_DEBUGLOG(SPDK_LOG_BDEV_IBOF, "read %lu bytes from offset %#lx\n",
		      len, offset);

	task->status = SPDK_BDEV_IO_STATUS_SUCCESS;
	task->num_outstanding = iovcnt;

	for (i = 0; i < iovcnt; i++) {
		res = spdk_copy_submit(__copy_task_from_ibof_task(task),
				       ch, iov[i].iov_base,
				       src, iov[i].iov_len, ibof_done);

		if (res != 0) {
			ibof_done(__copy_task_from_ibof_task(task), res);
		}

		src += iov[i].iov_len;
		len -= iov[i].iov_len;
	}
}

static void
bdev_ibof_ramdisk_dma_readv(struct ibof_disk *mdisk, struct spdk_io_channel *ch,
		  struct ibof_task *task,
		  struct iovec *iov, int iovcnt, size_t len, uint64_t offset)
{
	int64_t res = 0;
	void *src = mdisk->ibof_buf + offset;
	int i;

	if (bdev_malloc_check_iov_len(iov, iovcnt, len)) {
		spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task),
				      SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}

	SPDK_DEBUGLOG(bdev_malloc, "read %lu bytes from offset %#lx\n",
		      len, offset);

	task->status = SPDK_BDEV_IO_STATUS_SUCCESS;
	task->num_outstanding = iovcnt;

	for (i = 0; i < iovcnt; i++) {
		res = spdk_accel_submit_copy(ch, iov[i].iov_base,
					     src, iov[i].iov_len, ibof_done, task);

		if (res != 0) {
			ibof_done(task, res);
		}

		src += iov[i].iov_len;
		len -= iov[i].iov_len;
	}
}

static void
bdev_ibof_ramdisk_writev(struct ibof_disk *mdisk, struct spdk_io_channel *ch,
		   struct ibof_task *task,
		   struct iovec *iov, int iovcnt, size_t len, uint64_t offset)
{
	int64_t res = 0;
	void *dst = mdisk->ibof_buf + offset;
	int i;

	if (bdev_ibof_check_iov_len(iov, iovcnt, len)) {
		spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task),
				      SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}

	SPDK_DEBUGLOG(SPDK_LOG_BDEV_IBOF, "wrote %lu bytes to offset %#lx\n",
		      len, offset);

	task->status = SPDK_BDEV_IO_STATUS_SUCCESS;
	task->num_outstanding = iovcnt;

	for (i = 0; i < iovcnt; i++) {
		res = spdk_copy_submit(__copy_task_from_ibof_task(task),
				       ch, dst, iov[i].iov_base,
				       iov[i].iov_len, ibof_done);

		if (res != 0) {
			ibof_done(__copy_task_from_ibof_task(task), res);
		}

		dst += iov[i].iov_len;
	}
}

static void
bdev_ibof_ramdisk_dma_writev(struct ibof_disk *mdisk, struct spdk_io_channel *ch,
		   struct ibof_task *task,
		   struct iovec *iov, int iovcnt, size_t len, uint64_t offset)
{
	int64_t res = 0;
	void *dst = mdisk->ibof_buf + offset;
	int i;

	if (bdev_ibof_check_iov_len(iov, iovcnt, len)) {
		spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task),
				      SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}

	SPDK_DEBUGLOG(SPDK_LOG_BDEV_IBOF, "wrote %lu bytes to offset %#lx\n",
		      len, offset);

	task->status = SPDK_BDEV_IO_STATUS_SUCCESS;
	task->num_outstanding += iovcnt;

	struct ioat_io_channel* ioat_ch = spdk_io_channel_get_ctx(ch);
	for (i = 0; i < iovcnt; i++) {
		res = spdk_ioat_submit_copy(ioat_ch->ioat_ch,
			__copy_task_from_ibof_task(task),
			ibof_dma_done,
			dst,
			iov[i].iov_base,
			iov[i].iov_len);

		if (res != 0) {
			ibof_done(__copy_task_from_ibof_task(task), res);
		}

		dst += iov[i].iov_len;
	}
}

static int
bdev_ibof_unmap(struct ibof_disk *mdisk,
		  struct spdk_io_channel *ch,
		  struct ibof_task *task,
		  uint64_t offset,
		  uint64_t byte_count)
{
	task->status = SPDK_BDEV_IO_STATUS_SUCCESS;
	task->num_outstanding += 1;

	return spdk_copy_submit_fill(__copy_task_from_ibof_task(task), ch,
				     mdisk->ibof_buf + offset, 0, byte_count, ibof_done);
}

static int64_t
bdev_ibof_flush(struct ibof_disk *mdisk, struct ibof_task *task,
		  uint64_t offset, uint64_t nbytes)
{
	spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task), SPDK_BDEV_IO_STATUS_SUCCESS);

	return 0;
}

static int
bdev_ibof_reset(struct ibof_disk *mdisk, struct ibof_task *task)
{
	spdk_bdev_io_complete(spdk_bdev_io_from_ctx(task), SPDK_BDEV_IO_STATUS_SUCCESS);

	return 0;
}

static int _bdev_ibof_ramdisk_rw(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	uint32_t block_size = bdev_io->bdev->blocklen;

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
		if (bdev_io->u.bdev.iovs[0].iov_base == NULL) {
			assert(bdev_io->u.bdev.iovcnt == 1);
			bdev_io->u.bdev.iovs[0].iov_base =
				((struct ibof_disk *)bdev_io->bdev->ctxt)->ibof_buf +
				bdev_io->u.bdev.offset_blocks * block_size;
			bdev_io->u.bdev.iovs[0].iov_len = bdev_io->u.bdev.num_blocks * block_size;
			spdk_bdev_io_complete(spdk_bdev_io_from_ctx(bdev_io->driver_ctx),
					      SPDK_BDEV_IO_STATUS_SUCCESS);
			return 0;
		}

		bdev_ibof_ramdisk_readv((struct ibof_disk *)bdev_io->bdev->ctxt,
				  ch,
				  (struct ibof_task *)bdev_io->driver_ctx,
				  bdev_io->u.bdev.iovs,
				  bdev_io->u.bdev.iovcnt,
				  bdev_io->u.bdev.num_blocks * block_size,
				  bdev_io->u.bdev.offset_blocks * block_size);
		return 0;

	case SPDK_BDEV_IO_TYPE_WRITE:
		bdev_ibof_ramdisk_writev((struct ibof_disk *)bdev_io->bdev->ctxt,
				   ch,
				   (struct ibof_task *)bdev_io->driver_ctx,
				   bdev_io->u.bdev.iovs,
				   bdev_io->u.bdev.iovcnt,
				   bdev_io->u.bdev.num_blocks * block_size,
				   bdev_io->u.bdev.offset_blocks * block_size);
		return 0;
	}
	return 0;
}

static int _bdev_ibof_ramdisk_dma_rw(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	uint32_t block_size = bdev_io->bdev->blocklen;

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
		if (bdev_io->u.bdev.iovs[0].iov_base == NULL) {
			assert(bdev_io->u.bdev.iovcnt == 1);
			bdev_io->u.bdev.iovs[0].iov_base =
				((struct ibof_disk *)bdev_io->bdev->ctxt)->ibof_buf +
				bdev_io->u.bdev.offset_blocks * block_size;
			bdev_io->u.bdev.iovs[0].iov_len = bdev_io->u.bdev.num_blocks * block_size;
			spdk_bdev_io_complete(spdk_bdev_io_from_ctx(bdev_io->driver_ctx),
					      SPDK_BDEV_IO_STATUS_SUCCESS);
			return 0;
		}

		bdev_ibof_ramdisk_dma_readv((struct ibof_disk *)bdev_io->bdev->ctxt,
				  ch,
				  (struct ibof_task *)bdev_io->driver_ctx,
				  bdev_io->u.bdev.iovs,
				  bdev_io->u.bdev.iovcnt,
				  bdev_io->u.bdev.num_blocks * block_size,
				  bdev_io->u.bdev.offset_blocks * block_size);
		return 0;

	case SPDK_BDEV_IO_TYPE_WRITE:
		bdev_ibof_ramdisk_dma_writev((struct ibof_disk *)bdev_io->bdev->ctxt,
				   ch,
				   (struct ibof_task *)bdev_io->driver_ctx,
				   bdev_io->u.bdev.iovs,
				   bdev_io->u.bdev.iovcnt,
				   bdev_io->u.bdev.num_blocks * block_size,
				   bdev_io->u.bdev.offset_blocks * block_size);
		return 0;
	}
	return 0;
}

static void bdev_ibof_io_complete(struct ibof_io* io, int status)
{
    if(io->context){
        struct spdk_bdev_io* bio = (struct spdk_bdev_io*)io->context;
        int ret = (status == IBOF_IO_STATUS_SUCCESS) ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
        spdk_bdev_io_complete(bio, ret);
    }

    if (READ == io->ioType) {
        AIRLOG(LAT_VOLUME_READ, AIR_END, io->volume_id, (uint64_t)io->context);
    }
    else if (WRITE == io->ioType) {
        AIRLOG(LAT_VOLUME_WRITE, AIR_END, io->volume_id, (uint64_t)io->context);
    }

    free(io);
}

static int bdev_ibof_eventq_readv(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
		struct spdk_bdev_io *bio,
		struct iovec *iov, int iovcnt, uint64_t byte_length, uint64_t byte_offset) 
{
	SPDK_DEBUGLOG(SPDK_LOG_BDEV_IBOF, "read %lu blocks with offset %#lx (vid=%d)\n",
		      byte_length, byte_offset, ibdev->volume.id);


	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if(submit){
		struct ibof_io* io = (struct ibof_io*)malloc(sizeof(struct ibof_io));
		if(io){
			io->ioType = READ;
			io->volume_id = ibdev->volume.id;
			io->iov = iov;
			io->iovcnt = iovcnt;
			io->length = byte_length;
			io->offset = byte_offset;
			io->context = (void*)bio;
			io->complete_cb = bdev_ibof_io_complete;			
			return submit(io);
		}
	}
	else{
		SPDK_NOTICELOG("READ no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}

static int bdev_ibof_eventq_writev(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
		 struct spdk_bdev_io *bio,
		 struct iovec *iov, int iovcnt, uint64_t byte_length, uint64_t byte_offset) 
{
	SPDK_DEBUGLOG(SPDK_LOG_BDEV_IBOF, "write %lu blocks with offset %#lx (vid=%d)\n",
		      byte_length, byte_offset, ibdev->volume.id);

	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if(submit){
		struct ibof_io* io = (struct ibof_io*)malloc(sizeof(struct ibof_io));
		if(io){
			io->ioType = WRITE;
			io->volume_id = ibdev->volume.id;
			io->iov = iov;
			io->iovcnt = iovcnt;
			io->length = byte_length;
			io->offset = byte_offset;
			io->context = (void*)bio;
			io->complete_cb = bdev_ibof_io_complete;					
			return submit(io);
		}
	}
	else{
		SPDK_NOTICELOG("WRITE no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}

static int bdev_ibof_eventq_flush(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
		 struct spdk_bdev_io *bio) 
{
	SPDK_DEBUGLOG(SPDK_LOG_BDEV_IBOF, "flush with (vid=%d)\n", ibdev->volume.id);

	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if(submit){
		struct ibof_io* io = (struct ibof_io*)malloc(sizeof(struct ibof_io));
		if(io){
			io->ioType = FLUSH;
			io->volume_id = ibdev->volume.id;
			io->iov = NULL;
 			io->iovcnt = 0;
 			io->length = 0;
 			io->offset = 0;
			io->context = (void*)bio;
			io->complete_cb = bdev_ibof_io_complete;
			return submit(io);
		}
	}
	else{
		SPDK_NOTICELOG("FLUSH no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}

static void bdev_ibof_get_buf_cb(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io, bool success)
{
	if(!success){
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
	}

	int ret;
	uint32_t block_size = bdev_io->bdev->blocklen;

	ret = bdev_ibof_eventq_readv((struct ibof_disk *)bdev_io->bdev->ctxt,
				   ch,
				   bdev_io,
				   bdev_io->u.bdev.iovs,
				   bdev_io->u.bdev.iovcnt,
				   bdev_io->u.bdev.num_blocks * block_size,
				   bdev_io->u.bdev.offset_blocks * block_size);
	if(spdk_likely(ret == 0)){
		return;
	}
	else if(ret == -ENOMEM){
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_NOMEM);
	}
	else {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
	}
}

static int _bdev_ibof_eventq_rw(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	uint32_t block_size = bdev_io->bdev->blocklen;

    uint32_t vol_id = ((struct ibof_disk*)bdev_io->bdev->ctxt)->volume.id;

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
        {
            AIRLOG(LAT_VOLUME_READ, AIR_BEGIN, vol_id, (uint64_t)bdev_io);
            spdk_bdev_io_get_buf(bdev_io, bdev_ibof_get_buf_cb,
                    bdev_io->u.bdev.num_blocks * block_size);
            return 0;
        }

	case SPDK_BDEV_IO_TYPE_WRITE:
        {
            AIRLOG(LAT_VOLUME_WRITE, AIR_BEGIN, vol_id, (uint64_t)bdev_io);
            return bdev_ibof_eventq_writev((struct ibof_disk *)bdev_io->bdev->ctxt,
                    ch,
                    bdev_io,
                    bdev_io->u.bdev.iovs,
                    bdev_io->u.bdev.iovcnt,
                    bdev_io->u.bdev.num_blocks * block_size,
                    bdev_io->u.bdev.offset_blocks * block_size);
        }
	}
	return -EINVAL;
}

static int _bdev_ibof_eventq_flush(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	return bdev_ibof_eventq_flush((struct ibof_disk *)bdev_io->bdev->ctxt,
		ch,
		bdev_io);
}

static int _bdev_ibof_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	uint32_t block_size = bdev_io->bdev->blocklen;

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
	case SPDK_BDEV_IO_TYPE_WRITE:
		{
			struct ibof_disk* disk = (struct ibof_disk *)bdev_io->bdev->ctxt;
			if(disk->volume.ibof_bdev_io){
				return disk->volume.ibof_bdev_io(ch, bdev_io);
			}
			else{
				return -1;
			}
		}
	case SPDK_BDEV_IO_TYPE_RESET:
		return bdev_ibof_reset((struct ibof_disk *)bdev_io->bdev->ctxt,
					 (struct ibof_task *)bdev_io->driver_ctx);
	case SPDK_BDEV_IO_TYPE_FLUSH:
		{
			struct ibof_disk* disk = (struct ibof_disk *)bdev_io->bdev->ctxt;
			if(disk->volume.ibof_bdev_flush){
				return disk->volume.ibof_bdev_flush(ch, bdev_io);
			}
			else{
				return bdev_ibof_flush((struct ibof_disk *)bdev_io->bdev->ctxt,
							 (struct ibof_task *)bdev_io->driver_ctx,
							 bdev_io->u.bdev.offset_blocks * block_size,
							 bdev_io->u.bdev.num_blocks * block_size);
			}
		}
	case SPDK_BDEV_IO_TYPE_UNMAP:
		return bdev_ibof_unmap((struct ibof_disk *)bdev_io->bdev->ctxt,
					 ch,
					 (struct ibof_task *)bdev_io->driver_ctx,
					 bdev_io->u.bdev.offset_blocks * block_size,
					 bdev_io->u.bdev.num_blocks * block_size);

	case SPDK_BDEV_IO_TYPE_WRITE_ZEROES:
		/* bdev_ibof_unmap is implemented with a call to mem_cpy_fill which zeroes out all of the requested bytes. */
		return bdev_ibof_unmap((struct ibof_disk *)bdev_io->bdev->ctxt,
					 ch,
					 (struct ibof_task *)bdev_io->driver_ctx,
					 bdev_io->u.bdev.offset_blocks * block_size,
					 bdev_io->u.bdev.num_blocks * block_size);

	default:
		return -1;
	}
	return 0;
}

static void bdev_ibof_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	if (_bdev_ibof_submit_request(ch, bdev_io) != 0) {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
	}
}

static struct spdk_io_channel *
bdev_ibof_get_io_channel(void *ctx)
{
	return spdk_get_io_channel(&g_ibof_disks);
}

static void
bdev_ibof_write_json_config(struct spdk_bdev *bdev, struct spdk_json_write_ctx *w)
{
	char uuid_str[SPDK_UUID_STRING_LEN];

	spdk_json_write_object_begin(w);

	spdk_json_write_named_string(w, "method", "construct_ibof_bdev");

	spdk_json_write_named_object_begin(w, "params");
	spdk_json_write_named_string(w, "name", bdev->name);
	spdk_json_write_named_uint64(w, "num_blocks", bdev->blockcnt);
	spdk_json_write_named_uint32(w, "block_size", bdev->blocklen);
	spdk_uuid_fmt_lower(uuid_str, sizeof(uuid_str), &bdev->uuid);
	spdk_json_write_named_string(w, "uuid", uuid_str);

	spdk_json_write_object_end(w);
	spdk_json_write_object_end(w);
}

static int
bdev_ibof_poll(void* arg)
{
	unvmf_complete_handler complete = (unvmf_complete_handler)arg;
	if (complete){
		complete();
	}
	else{
		SPDK_ERRLOG("uNVMf Complete Handler does not exist\n");
	}
	return 0;
}

static const struct spdk_bdev_fn_table ibof_fn_table = {
	.destruct		= bdev_ibof_destruct,
	.submit_request		= bdev_ibof_submit_request,
	.io_type_supported	= bdev_ibof_io_type_supported,
	.get_io_channel		= bdev_ibof_get_io_channel,
	.write_config_json	= bdev_ibof_write_json_config,
};

struct spdk_bdev *create_ibof_disk(const char* volume_name, uint32_t volume_id, const struct spdk_uuid* bdev_uuid, uint64_t num_blocks, uint32_t block_size, bool volume_type_in_memory)
{
	struct ibof_disk	*mdisk;
	int			rc;

	if (num_blocks == 0) {
		SPDK_ERRLOG("Disk must be more than 0 blocks\n");
		return NULL;
	}

	mdisk = spdk_dma_zmalloc(sizeof(*mdisk), 0, NULL);
	if (!mdisk) {
		SPDK_ERRLOG("mdisk spdk_dma_zibof() failed\n");
		return NULL;
	}

	//NOTE: VolumeTypeInMemory in configuration will make nvmf target work as debug mode.
	//volume_type_in_memory = true;
	if(volume_type_in_memory){
		if(spdk_check_ioat_initialized()){
			SPDK_NOTICELOG("Initialize ramdisk using DMA engine\n");
			mdisk->volume.ibof_bdev_io = _bdev_ibof_ramdisk_dma_rw;
		}
		else{
			SPDK_NOTICELOG("Initialize ramdisk using COPY engine\n");
			mdisk->volume.ibof_bdev_io = _bdev_ibof_ramdisk_rw;
		}
		mdisk->ibof_buf = spdk_dma_zmalloc(num_blocks * block_size, 2*MB, NULL);
	}
	else{
		uint64_t volume_buffer_size = 2*MB;
		mdisk->volume.ibof_bdev_io = _bdev_ibof_eventq_rw;
		mdisk->volume.ibof_bdev_flush = _bdev_ibof_eventq_flush;
		mdisk->ibof_buf = spdk_dma_zmalloc(volume_buffer_size, 2*MB , NULL);
	}
	if (!mdisk->ibof_buf) {
		SPDK_ERRLOG("ibof_buf buffer: spdk_dma_zmalloc() failed\n");
		ibof_disk_free(mdisk);
		return NULL;
	}

	if(volume_name){
		mdisk->disk.name = strdup(volume_name);
	}
	else{
		mdisk->disk.name = spdk_sprintf_alloc("Volume%d", ibof_disk_count);
	}
	if (!mdisk->disk.name) {
		ibof_disk_free(mdisk);
		return NULL;
	}
	mdisk->disk.product_name = "iBoF Volume";
	strncpy(mdisk->volume.name,mdisk->disk.name,strlen(mdisk->disk.name));
	mdisk->volume.id = volume_id;
	mdisk->volume.size_mb = (num_blocks * block_size) / MB;
	mdisk->disk.write_cache = 1;
	mdisk->disk.blocklen = block_size;
	mdisk->disk.blockcnt = num_blocks;
	if(bdev_uuid) {
		mdisk->disk.uuid = *bdev_uuid;
	}
	else{
		spdk_uuid_generate(&mdisk->disk.uuid);
	}
	mdisk->disk.ctxt = mdisk;
	mdisk->disk.fn_table = &ibof_fn_table;
	mdisk->disk.module = &ibof_if;
	rc = spdk_bdev_register(&mdisk->disk);
	if (rc) {
		ibof_disk_free(mdisk);
		return NULL;
	}
	TAILQ_INSERT_TAIL(&g_ibof_disks, mdisk, link);
	ibof_disk_count++;

	SPDK_NOTICELOG("iBoF_Volume(volume_id=%u, size_mb=%ld) has created. volume_type_in_memory=%d\n",
			mdisk->volume.id, mdisk->volume.size_mb, volume_type_in_memory);
	return &mdisk->disk;
}
int get_ibof_volume_id(struct spdk_bdev *bdev)
{
	return ((struct ibof_disk*)bdev->ctxt)->volume.id;
}

void
delete_ibof_disk(struct spdk_bdev *bdev, spdk_delete_ibof_complete cb_fn, void *cb_arg)
{
	if (!bdev || bdev->module != &ibof_if) {
		cb_fn(cb_arg, -ENODEV);
		return;
	}

	SPDK_NOTICELOG("ibof_volume(%s) has deleted", spdk_bdev_get_name(bdev));
	spdk_bdev_unregister(bdev, cb_fn, cb_arg);
	ibof_disk_count--;
}

static int
ibof_bdev_create_cb(void *io_device, void *ctx_buf)
{
    return 0;
}

static void
ibof_bdev_destroy_cb(void *io_device, void *ctx_buf)
{
}

static int bdev_ibof_initialize(void)
{
	struct spdk_conf_section *sp = spdk_conf_find_section(NULL, "Volume");
	int rc = 0;
	struct spdk_bdev *bdev = NULL;
	spdk_io_device_register(&g_ibof_disks, ibof_bdev_create_cb, ibof_bdev_destroy_cb,
            sizeof(struct ibof_io_channel),
            "ibof_bdev");

	if (sp != NULL) {
		int block_size = 0;
		uint32_t volume_id = spdk_conf_section_get_intval(sp, "VolumeID");
		uint64_t volume_size_mb = spdk_conf_section_get_intval(sp, "VolumeSizeInMB");
		bool volume_type_in_memory = spdk_conf_section_get_boolval(sp, "VolumeTypeInMemory", false);

		if ((volume_size_mb < 1)) {
			SPDK_ERRLOG("Volume section present, but no devices specified\n");
			goto end;
		}
		if(volume_id > NR_MAX_VOLUME){
			volume_id = 0;
		}
		volume_size_mb *= MB;
		block_size = 512;
		bdev = create_ibof_disk(NULL, volume_id, NULL, volume_size_mb / block_size, block_size, volume_type_in_memory);
		if (bdev == NULL) {
			SPDK_ERRLOG("Could not create ibof disk\n");
			rc = EINVAL;
			goto end;
		}
	}

end:
	return rc;
}

static void bdev_ibof_register_poller(void* arg1, void* arg2)
{
	SPDK_NOTICELOG("%s: current_core=%d last_core=%d\n", __FUNCTION__,
		spdk_env_get_current_core(), spdk_env_get_last_core());
     
	uint32_t current_core = spdk_env_get_current_core();
	if (NULL == per_lcore_ibof_poller){
		per_lcore_ibof_poller = spdk_poller_register(bdev_ibof_poll, arg1, 0);
		if (spdk_likely(NULL != per_lcore_ibof_poller)){
			SPDK_NOTICELOG("Registered unvmf bdev_ibof poller to "\
					"frontend io handler(#%u)\n", current_core);
		}
		else{
			SPDK_ERRLOG("Failed to register unvmf bdev_ibof poller "\
					"to frontend io handler(#%u)\n", current_core);
		}
	}

	if (current_core != spdk_env_get_last_core()){
		struct spdk_event* e =
			spdk_event_allocate(spdk_env_get_next_core(current_core),
					bdev_ibof_register_poller, arg1, NULL);
		spdk_event_call(e);
	}
	per_lcore_poller_ref_count++;
}
#if defined QOS_ENABLED_FE
void spdk_bdev_ibof_register_io_handler(const char* bdev_name,
	unvmf_io_handler handler, const char* nqn, uint32_t nqn_id)
#else
void spdk_bdev_ibof_register_io_handler(const char* bdev_name,
	unvmf_io_handler handler, const char* nqn)
#endif
{
	struct spdk_bdev* bdev = spdk_bdev_get_by_name(bdev_name);
	if(bdev){
		struct ibof_disk* disk = (struct ibof_disk*)bdev->ctxt;
		if(disk){
			if(disk->volume.unvmf_io.submit){
				if (disk->volume.unvmf_io.submit != handler.submit){
					SPDK_ERRLOG("overwriting io submit handler with "\
						"0x%lx for bdev=%s\n", 
						(uint64_t)handler.submit, bdev_name);
				}
			}
			else{
				disk->volume.unvmf_io.submit = handler.submit;
			}

			if(disk->volume.unvmf_io.complete){
				if (disk->volume.unvmf_io.complete != handler.complete){
					SPDK_ERRLOG("overwriting io complete handler with "\
						"0x%lx for bdev=%s\n",
						(uint64_t)handler.complete, bdev_name);
				}
			}
			else{
				disk->volume.unvmf_io.complete = handler.complete;	
			}	
			if(nqn){
				strncpy(disk->volume.nqn, nqn, sizeof(disk->volume.nqn));
				disk->volume.nqn[sizeof(disk->volume.nqn)-1] = '\0';
#if defined QOS_ENABLED_FE
				disk->volume.nqn_id = nqn_id;
#endif
			}
			struct spdk_event* e = spdk_event_allocate(
						spdk_env_get_first_core(),
						bdev_ibof_register_poller, handler.complete, NULL);
			spdk_event_call(e);
		}
	}
	else{
		SPDK_ERRLOG("fail to find bdev(%s)\n",bdev_name);
	}
}

static void bdev_ibof_unregister_poller(void* arg1, void* arg2)
{
	struct ibof_disk* disk = (struct ibof_disk*)arg1;
	uint32_t current_core = spdk_env_get_current_core();

	if (NULL != per_lcore_ibof_poller){
		per_lcore_poller_ref_count--;
		if (0 == per_lcore_poller_ref_count){
			spdk_poller_unregister(&per_lcore_ibof_poller);
			SPDK_NOTICELOG("Unregistered unvmf bdev_ibof poller from "\
				"frontend io handler(#%u)\n", current_core);
		}
	}
	if (current_core != spdk_env_get_last_core()){
		struct spdk_event* e =
			spdk_event_allocate(spdk_env_get_next_core(current_core),
					bdev_ibof_unregister_poller, disk, NULL);
		spdk_event_call(e);
	}
	else if (0 == per_lcore_poller_ref_count){
		if(disk){
			disk->volume.unvmf_io.submit = NULL;
			disk->volume.unvmf_io.complete = NULL;
			memset(disk->volume.nqn, 0, sizeof(disk->volume.nqn));
		}
	}
}

void spdk_bdev_ibof_unregister_io_handler(const char* bdev_name)
{
	struct spdk_bdev* bdev = spdk_bdev_get_by_name(bdev_name);
	if(bdev){
		struct ibof_disk* disk = (struct ibof_disk*)bdev->ctxt;
		struct spdk_event* e = spdk_event_allocate(spdk_env_get_first_core(), bdev_ibof_unregister_poller, disk, NULL);
		spdk_event_call(e);
	}
}

struct spdk_bdev* spdk_bdev_create_ibof_disk(const char* volume_name, uint32_t volume_id, const struct spdk_uuid* bdev_uuid, uint64_t num_blocks, uint32_t block_size, bool volume_type_in_memory)
{ 
	return create_ibof_disk(volume_name, volume_id, bdev_uuid, num_blocks, block_size, volume_type_in_memory);
}

void spdk_bdev_delete_ibof_disk(struct spdk_bdev *bdev, spdk_delete_ibof_complete cb_fn, void *cb_arg)
{
	return delete_ibof_disk(bdev, cb_fn, cb_arg);
}

const char* get_attached_subsystem_nqn(const char* bdev_name)
{
	struct spdk_bdev* bdev = spdk_bdev_get_by_name(bdev_name);
	if(bdev == NULL){
		SPDK_ERRLOG("Failed to get bdev(%s)'s nqn : bdev does not exist\n", bdev_name);
		return NULL;
	}
	struct ibof_disk* disk = (struct ibof_disk*)bdev->ctxt;
	return disk->volume.nqn;
}
#if defined QOS_ENABLED_FE
uint32_t get_attached_subsystem_id(const char* bdev_name)
{
	struct spdk_bdev* bdev = spdk_bdev_get_by_name(bdev_name);
	if(bdev == NULL){
		SPDK_ERRLOG("Failed to get bdev(%s)'s nqn : bdev does not exist\n", bdev_name);
		return NULL;
	}
	struct ibof_disk* disk = (struct ibof_disk*)bdev->ctxt;
	return disk->volume.nqn_id;
}
#endif
SPDK_LOG_REGISTER_COMPONENT("bdev_ibof", SPDK_LOG_BDEV_IBOF)
