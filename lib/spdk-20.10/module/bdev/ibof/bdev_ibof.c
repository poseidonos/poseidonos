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
#include "spdk/json.h"
#include "spdk/thread.h"
#include "spdk/queue.h"
#include "spdk/string.h"
#include "spdk/likely.h"

#include "spdk/bdev_module.h"
#include "spdk/log.h"
#include "spdk/ibof.h"
#include "spdk/ibof_volume.h"
#include "spdk/ibof_nvmf.h"
#include "spdk/event.h"
#include "spdk/ioat.h"
#ifdef _ADMIN_ENABLED
#include "spdk/nvmf_transport.h"
#endif
#include "Air.h"

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

struct ibof_io_channel {
	struct spdk_poller		*poller;
	TAILQ_HEAD(, spdk_bdev_io)	io;
};

//struct ibof_io_channel {
//}

struct ioat_io_channel {
	struct spdk_ioat_chan	*ioat_ch;
	struct ioat_device	*ioat_dev;
	struct spdk_poller	*poller;
};

__thread __typeof__(struct spdk_poller *) per_lcore_ibof_poller;

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
	struct ibof_task *task = (struct ibof_task *)ref;

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

static TAILQ_HEAD(, ibof_disk) g_ibof_disks = TAILQ_HEAD_INITIALIZER(g_ibof_disks);

static uint32_t ibof_disk_count = 0;

static int bdev_ibof_initialize(void);
static void bdev_ibof_get_spdk_running_config(FILE *fp);

static int
bdev_ibof_get_ctx_size(void)
{
	return sizeof(struct ibof_task);
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
#ifdef _ADMIN_ENABLED
	case SPDK_BDEV_IO_TYPE_NVME_ADMIN:
#endif
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
			ibof_buffer_size = (mdisk->disk.blocklen * mdisk->disk.blockcnt) / (1024 * 1024);
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

	SPDK_DEBUGLOG(bdev_ibof, "read %lu bytes from offset %#lx\n",
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

	SPDK_DEBUGLOG(bdev_ibof, "wrote %lu bytes to offset %#lx\n",
		      len, offset);

	task->status = SPDK_BDEV_IO_STATUS_SUCCESS;
	task->num_outstanding = iovcnt;

	for (i = 0; i < iovcnt; i++) {
		res = spdk_accel_submit_copy(ch, dst, iov[i].iov_base,
					     iov[i].iov_len, ibof_done, task);

		if (res != 0) {
			ibof_done(task, res);
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
	task->num_outstanding = 1;

	return spdk_accel_submit_fill(ch, mdisk->ibof_buf + offset, 0,
				      byte_count, ibof_done, task);
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

static void bdev_ibof_io_complete(struct ibof_io *io, int status)
{
	if (io->context) {
		struct spdk_bdev_io *bio = (struct spdk_bdev_io *)io->context;
		int ret = (status == IBOF_IO_STATUS_SUCCESS) ? SPDK_BDEV_IO_STATUS_SUCCESS :
			  SPDK_BDEV_IO_STATUS_FAILED;
		spdk_bdev_io_complete(bio, ret);
	}

	if (READ == io->ioType) {
		AIRLOG(LAT_BDEV_READ, io->volume_id, 1, (uint64_t)io->context);
	} else if (WRITE == io->ioType) {
		AIRLOG(LAT_BDEV_WRITE, io->volume_id, 1, (uint64_t)io->context);
	}

	free(io);
}

static int bdev_ibof_eventq_readv(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
				  struct spdk_bdev_io *bio,
				  struct iovec *iov, int iovcnt, uint64_t byte_length, uint64_t byte_offset)
{
	SPDK_DEBUGLOG(bdev_ibof, "read %lu blocks with offset %#lx (vid=%d)\n",
		      byte_length, byte_offset, ibdev->volume.id);


	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if (submit) {
		struct ibof_io *io = (struct ibof_io *)malloc(sizeof(struct ibof_io));
		if (io) {
			io->ioType = READ;
			io->volume_id = ibdev->volume.id;
			io->iov = iov;
			io->iovcnt = iovcnt;
			io->length = byte_length;
			io->offset = byte_offset;
			io->context = (void *)bio;
			io->arrayName = ibdev->volume.array_name;
			io->complete_cb = bdev_ibof_io_complete;
			struct spdk_bdev_io *bdev_io = (struct spdk_bdev_io *)io->context;
			assert(spdk_get_thread() == spdk_bdev_io_get_thread(bdev_io));
			return submit(io);
		}
	} else {
		SPDK_NOTICELOG("READ no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}

static int bdev_ibof_eventq_writev(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
				   struct spdk_bdev_io *bio,
				   struct iovec *iov, int iovcnt, uint64_t byte_length, uint64_t byte_offset)
{
	SPDK_DEBUGLOG(bdev_ibof, "write %lu blocks with offset %#lx (vid=%d)\n",
		      byte_length, byte_offset, ibdev->volume.id);

	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if (submit) {
		struct ibof_io *io = (struct ibof_io *)malloc(sizeof(struct ibof_io));
		if (io) {
			io->ioType = WRITE;
			io->volume_id = ibdev->volume.id;
			io->iov = iov;
			io->iovcnt = iovcnt;
			io->length = byte_length;
			io->offset = byte_offset;
			io->context = (void *)bio;
			io->arrayName = ibdev->volume.array_name;
			io->complete_cb = bdev_ibof_io_complete;
			struct spdk_bdev_io *bdev_io = (struct spdk_bdev_io *)io->context;
			assert(spdk_get_thread() == spdk_bdev_io_get_thread(bdev_io));
			return submit(io);
		}
	} else {
		SPDK_NOTICELOG("WRITE no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}

static int bdev_ibof_eventq_flush(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
				  struct spdk_bdev_io *bio)
{
	SPDK_DEBUGLOG(bdev_ibof, "flush with (vid=%d)\n", ibdev->volume.id);

	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if (submit) {
		struct ibof_io *io = (struct ibof_io *)malloc(sizeof(struct ibof_io));
		if (io) {
			io->ioType = FLUSH;
			io->volume_id = ibdev->volume.id;
			io->iov = NULL;
			io->iovcnt = 0;
			io->length = 0;
			io->offset = 0;
			io->context = (void *)bio;
			io->arrayName = ibdev->volume.array_name;
			io->complete_cb = bdev_ibof_io_complete;
			return submit(io);
		}
	} else {
		SPDK_NOTICELOG("FLUSH no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}
#ifdef _ADMIN_ENABLED
static int bdev_ibof_eventq_get_smart_log_page(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
		struct spdk_nvme_cmd *cmd, struct spdk_bdev_io *bio)
{
	unvmf_submit_handler submit = ibdev->volume.unvmf_io.submit;
	if (submit) {
		struct ibof_io *io = (struct ibof_io *)malloc(sizeof(struct ibof_io));
		if (io) {
			io->ioType = GET_LOG_PAGE;
			io->volume_id = ibdev->volume.id;
			io->context = (void *)bio;
			io->complete_cb = bdev_ibof_io_complete;
			return submit(io);
		}
	} else {
		SPDK_NOTICELOG("ADMIN no submit handler %s\n", ibdev->disk.name);
	}
	spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_FAILED);
	return 0;
}
static int bdev_ibof_eventq_get_log_page(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
		struct spdk_nvme_cmd *cmd, struct spdk_bdev_io *bio)
{
	uint8_t lid;
	lid = cmd->cdw10 & 0xFF;
	switch (lid) {
	case SPDK_NVME_LOG_HEALTH_INFORMATION:
		bdev_ibof_eventq_get_smart_log_page(ibdev, ch, cmd, bio);
		return 0;
	default:
		spdk_bdev_io_complete(bio, SPDK_BDEV_IO_STATUS_SUCCESS);
		return 0;
	}
	return -EINVAL;
}
static int bdev_ibof_eventq_admin(struct ibof_disk *ibdev, struct spdk_io_channel *ch,
				  struct spdk_bdev_io *bio)
{
	SPDK_DEBUGLOG(bdev_ibof, "admin command handling (vid=%d)\n",
		      ibdev->volume.id);

	void *caller_context = (bio->internal.caller_ctx);
	struct spdk_nvmf_request *req = (struct spdk_nvmf_request *)caller_context;
	struct spdk_nvme_cmd cmd = (struct spdk_nvme_cmd)(req->cmd->nvme_cmd);
	switch (cmd.opc) {
	case SPDK_NVME_OPC_GET_LOG_PAGE:
		bdev_ibof_eventq_get_log_page(ibdev, ch, &cmd, bio);
		return 0;
	default:
		return 0;
	}
	return -EINVAL;
}

#endif

static void bdev_ibof_get_buf_cb(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io,
				 bool success)
{
	if (!success) {
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
	if (spdk_likely(ret == 0)) {
		return;
	} else if (ret == -ENOMEM) {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_NOMEM);
	} else {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
	}
}

static int _bdev_ibof_eventq_rw(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	uint32_t block_size = bdev_io->bdev->blocklen;

	uint32_t vol_id = ((struct ibof_disk *)bdev_io->bdev->ctxt)->volume.id;

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ: {
		AIRLOG(LAT_BDEV_READ, vol_id, 0, (uint64_t)bdev_io);
		spdk_bdev_io_get_buf(bdev_io, bdev_ibof_get_buf_cb,
				     bdev_io->u.bdev.num_blocks * block_size);
		return 0;
	}

	case SPDK_BDEV_IO_TYPE_WRITE: {
		AIRLOG(LAT_BDEV_WRITE, vol_id, 0, (uint64_t)bdev_io);
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
#ifdef _ADMIN_ENABLED
static int _bdev_ibof_eventq_admin(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	return bdev_ibof_eventq_admin((struct ibof_disk *)bdev_io->bdev->ctxt,
				      ch,
				      bdev_io);
}
#endif

static int _bdev_ibof_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	uint32_t block_size = bdev_io->bdev->blocklen;

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
	case SPDK_BDEV_IO_TYPE_WRITE: {
		struct ibof_disk *disk = (struct ibof_disk *)bdev_io->bdev->ctxt;
		if (disk->volume.ibof_bdev_io) {
			return disk->volume.ibof_bdev_io(ch, bdev_io);
		} else {
			return -1;
		}
	}
	case SPDK_BDEV_IO_TYPE_RESET:
		return bdev_ibof_reset((struct ibof_disk *)bdev_io->bdev->ctxt,
				       (struct ibof_task *)bdev_io->driver_ctx);
	case SPDK_BDEV_IO_TYPE_FLUSH: {
		struct ibof_disk *disk = (struct ibof_disk *)bdev_io->bdev->ctxt;
		if (disk->volume.ibof_bdev_flush) {
			return disk->volume.ibof_bdev_flush(ch, bdev_io);
		} else {
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
#ifdef _ADMIN_ENABLED
	case SPDK_BDEV_IO_TYPE_NVME_ADMIN: {
		struct ibof_disk *disk = (struct ibof_disk *)bdev_io->bdev->ctxt;
		if (disk->volume.ibof_bdev_admin) {
			return disk->volume.ibof_bdev_admin(ch, bdev_io);
		} else {
			return -1;
		}
	}
#endif
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
bdev_ibof_poll(void *arg)
{
	unvmf_complete_handler complete = (unvmf_complete_handler)arg;
	if (complete) {
		complete();
	} else {
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


static int ibof_bdev_create_cb(void *io_device, void *ctx_buf);
static void ibof_bdev_destroy_cb(void *io_device, void *ctx_buf);

struct spdk_bdev *create_ibof_disk(const char *volume_name, uint32_t volume_id,
				   const struct spdk_uuid *bdev_uuid, uint64_t num_blocks, uint32_t block_size,
				   bool volume_type_in_memory, const char* array_name)
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

	// NOTE: VolumeTypeInMemory in configuration will make nvmf target work as debug mode.
	if (volume_type_in_memory) {
		SPDK_NOTICELOG("Initialize ramdisk using COPY engine, Please setting accel configuration in spdk\n");
		mdisk->volume.ibof_bdev_io = _bdev_ibof_ramdisk_rw;
		mdisk->ibof_buf = spdk_dma_zmalloc(num_blocks * block_size, 2 * MB, NULL);
	} else {
		uint64_t volume_buffer_size = 2 * MB;
		mdisk->volume.ibof_bdev_io = _bdev_ibof_eventq_rw;
		mdisk->volume.ibof_bdev_flush = _bdev_ibof_eventq_flush;
#ifdef _ADMIN_ENABLED
		mdisk->volume.ibof_bdev_admin = _bdev_ibof_eventq_admin;
#endif
		mdisk->ibof_buf = spdk_dma_zmalloc(volume_buffer_size, 2 * MB, NULL);
	}
	if (!mdisk->ibof_buf) {
		SPDK_ERRLOG("ibof_buf buffer: spdk_dma_zmalloc() failed\n");
		ibof_disk_free(mdisk);
		return NULL;
	}

	if (volume_name) {
		mdisk->disk.name = strdup(volume_name);
	} else {
		mdisk->disk.name = spdk_sprintf_alloc("Volume%d", ibof_disk_count);
	}
	if (!mdisk->disk.name) {
		ibof_disk_free(mdisk);
		return NULL;
	}
	mdisk->disk.product_name = "iBoF Volume";
	strncpy(mdisk->volume.name, mdisk->disk.name, strlen(mdisk->disk.name));
	uint32_t array_length = strlen(array_name);
	strncpy(mdisk->volume.array_name, array_name, array_length);
	mdisk->volume.array_name[array_length] = '\0';
	mdisk->volume.id = volume_id;
	mdisk->volume.size_mb = (num_blocks * block_size) / MB;
	mdisk->disk.write_cache = 1;
	mdisk->disk.blocklen = block_size;
	mdisk->disk.blockcnt = num_blocks;
	if (bdev_uuid) {
		mdisk->disk.uuid = *bdev_uuid;
	} else {
		spdk_uuid_generate(&mdisk->disk.uuid);
	}
	mdisk->disk.ctxt = mdisk;
	mdisk->disk.fn_table = &ibof_fn_table;
	mdisk->disk.module = &ibof_if;

	TAILQ_INSERT_TAIL(&g_ibof_disks, mdisk, link);
	ibof_disk_count++;

	rc = spdk_bdev_register(&mdisk->disk);
	if (rc) {
		ibof_disk_free(mdisk);
		return NULL;
	}

	SPDK_NOTICELOG("iBoF_Volume(volume_id=%u, size_mb=%ld) has created. volume_type_in_memory=%d\n",
		       mdisk->volume.id, mdisk->volume.size_mb, volume_type_in_memory);
	return &mdisk->disk;
}
int get_ibof_volume_id(struct spdk_bdev *bdev)
{
	return ((struct ibof_disk *)bdev->ctxt)->volume.id;
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

static void bdev_ibof_register_poller(void *arg1);

static int
ibof_bdev_create_cb(void *io_device, void *ctx_buf)
{
	return 0;
}

static void bdev_ibof_unregister_poller(void *arg1);

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
		if (volume_id > NR_MAX_VOLUME) {
			volume_id = 0;
		}
		volume_size_mb *= MB;
		block_size = 512;
		bdev = create_ibof_disk(NULL, volume_id, NULL, volume_size_mb / block_size, block_size,
					volume_type_in_memory, NULL);
		if (bdev == NULL) {
			SPDK_ERRLOG("Could not create ibof disk\n");
			rc = EINVAL;
			goto end;
		}
	}

end:
	return rc;
}

static void bdev_ibof_register_poller(void *arg1)
{
	SPDK_NOTICELOG("%s: current_core=%d \n", __FUNCTION__,
		       spdk_env_get_current_core());

	uint32_t current_core = spdk_env_get_current_core();
	if (NULL == per_lcore_ibof_poller) {
		per_lcore_ibof_poller = spdk_poller_register(bdev_ibof_poll, arg1, 0);
		if (spdk_likely(NULL != per_lcore_ibof_poller)) {
			SPDK_NOTICELOG("Registered unvmf bdev_ibof poller to "\
				       "frontend io handler(#%u)\n", current_core);
		} else {
			SPDK_ERRLOG("Failed to register unvmf bdev_ibof poller "\
				    "to frontend io handler(#%u)\n", current_core);
		}
	}
	if (current_core != spdk_env_get_last_core()) {
		send_msg_to_all_nvmf_thread(current_core, bdev_ibof_register_poller, arg1);
	}
	per_lcore_poller_ref_count++;
}

void spdk_bdev_ibof_register_io_handler(const char *bdev_name,
					unvmf_io_handler handler)
{
	struct spdk_bdev *bdev = spdk_bdev_get_by_name(bdev_name);
	if (bdev) {
		struct ibof_disk *disk = (struct ibof_disk *)bdev->ctxt;
		if (disk) {
			if (disk->volume.unvmf_io.submit) {
				if (disk->volume.unvmf_io.submit != handler.submit) {
					SPDK_ERRLOG("overwriting io submit handler with "\
						    "0x%lx for bdev=%s\n",
						    (uint64_t)handler.submit, bdev_name);
				}
			} else {
				disk->volume.unvmf_io.submit = handler.submit;
			}

			if (disk->volume.unvmf_io.complete) {
				if (disk->volume.unvmf_io.complete != handler.complete) {
					SPDK_ERRLOG("overwriting io complete handler with "\
						    "0x%lx for bdev=%s\n",
						    (uint64_t)handler.complete, bdev_name);
				}
			} else {
				disk->volume.unvmf_io.complete = handler.complete;
			}
			bdev_ibof_register_poller(handler.complete);
		}
	} else {
		SPDK_ERRLOG("fail to find bdev(%s)\n", bdev_name);
	}
}

static void bdev_ibof_unregister_poller(void *arg1)
{
	struct ibof_disk *disk = (struct ibof_disk *)arg1;
	uint32_t current_core = spdk_env_get_current_core();

	if (NULL != per_lcore_ibof_poller) {
		per_lcore_poller_ref_count--;
		if (0 == per_lcore_poller_ref_count) {
			spdk_poller_unregister(&per_lcore_ibof_poller);
			SPDK_NOTICELOG("Unregistered unvmf bdev_ibof poller from "\
				       "frontend io handler(#%u)\n", current_core);
		}
	}

	if (current_core != spdk_env_get_last_core()) {
		send_msg_to_all_nvmf_thread(current_core, bdev_ibof_unregister_poller, arg1);
	} else if (0 == per_lcore_poller_ref_count) {
		if (disk) {
			disk->volume.unvmf_io.submit = NULL;
			disk->volume.unvmf_io.complete = NULL;
		}
	}
}

void spdk_bdev_ibof_unregister_io_handler(const char *bdev_name)
{
	struct spdk_bdev *bdev = spdk_bdev_get_by_name(bdev_name);
	if (bdev) {
		struct ibof_disk *disk = (struct ibof_disk *)bdev->ctxt;
		if (disk) {
			bdev_ibof_unregister_poller(disk);
		}
	}
}

void set_ibof_volume_info(const char *bdev_name, const char *nqn, int nqn_id)
{
	struct spdk_bdev *bdev = spdk_bdev_get_by_name(bdev_name);
	if (bdev) {
		struct ibof_disk *disk = (struct ibof_disk *)bdev->ctxt;
		if (disk) {
			if (nqn) {
				strncpy(disk->volume.nqn, nqn, sizeof(disk->volume.nqn));
				disk->volume.nqn[sizeof(disk->volume.nqn) - 1] = '\0';
#if defined QOS_ENABLED_FE
				disk->volume.nqn_id = nqn_id;
#endif
			}
		}
	}
}

void reset_ibof_volume_info(const char *bdev_name)
{
	struct spdk_bdev *bdev = spdk_bdev_get_by_name(bdev_name);
	if (bdev) {
		struct ibof_disk *disk = (struct ibof_disk *)bdev->ctxt;
		if (disk) {
			memset(disk->volume.nqn, 0, sizeof(disk->volume.nqn));
		}
	}
}

struct spdk_bdev *spdk_bdev_create_ibof_disk(const char *volume_name, uint32_t volume_id,
		const struct spdk_uuid *bdev_uuid, uint64_t num_blocks, uint32_t block_size,
		bool volume_type_in_memory, const char *array_name)
{
	return create_ibof_disk(volume_name, volume_id, bdev_uuid, num_blocks, block_size,
				volume_type_in_memory, array_name);
}

void spdk_bdev_delete_ibof_disk(struct spdk_bdev *bdev, spdk_delete_ibof_complete cb_fn,
				void *cb_arg)
{
	return delete_ibof_disk(bdev, cb_fn, cb_arg);
}

void send_msg_to_all_nvmf_thread(uint32_t current_core, void *fn, void *arg1)
{
	uint32_t next_core = spdk_env_get_next_core(current_core);
	struct spdk_thread *thread = get_nvmf_thread_from_reactor(next_core);
	if (thread == NULL) {
		SPDK_ERRLOG("Failed to get nvmf thread from reactor(#%u)\n", current_core);
	}

	int success = spdk_thread_send_msg(thread, fn, arg1);
	if (0 != success) {
		SPDK_ERRLOG("Failed to send messag\n");
	}
}

const char *get_attached_subsystem_nqn(const char *bdev_name)
{
	struct spdk_bdev *bdev = spdk_bdev_get_by_name(bdev_name);
	if (bdev == NULL) {
		SPDK_ERRLOG("Failed to get bdev(%s)'s nqn : bdev does not exist\n", bdev_name);
		return NULL;
	}
	struct ibof_disk *disk = (struct ibof_disk *)bdev->ctxt;
	return disk->volume.nqn;
}
#if defined QOS_ENABLED_FE
uint32_t get_attached_subsystem_id(const char *bdev_name)
{
	struct spdk_bdev *bdev = spdk_bdev_get_by_name(bdev_name);
	if (bdev == NULL) {
		SPDK_ERRLOG("Failed to get bdev(%s)'s nqn : bdev does not exist\n", bdev_name);
		return NULL;
	}
	struct ibof_disk *disk = (struct ibof_disk *)bdev->ctxt;
	return disk->volume.nqn_id;
}
#endif
SPDK_LOG_REGISTER_COMPONENT(bdev_ibof)
