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

#ifndef SPDK_IBOF_VOLUME_H_
#define SPDK_IBOF_VOLUME_H_

#include "spdk/stdinc.h"
#include "nvmf_spec.h"
#include "ibof.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IBOF_IO_STATUS_SUCCESS (0)
#define IBOF_IO_STATUS_FAIL (-1)

#define VOLUME_NAME_MAX_LEN (255)
#define NR_MAX_VOLUME (256)

enum IO_TYPE {
	READ = 0,
	WRITE,
#if defined NVMe_FLUSH_HANDLING
        FLUSH
#endif
};

/*
 * register the IO submit/compete callback that links uNVMf ibof_bdev and Frontend
 */
typedef int (*unvmf_submit_handler)(struct ibof_io* io);
typedef void (*unvmf_complete_handler)(void);
typedef struct unvmf_io_handler{
	unvmf_submit_handler submit;
	unvmf_complete_handler complete;
}unvmf_io_handler;
#if defined QOS_ENABLED_FE
uint32_t get_attached_subsystem_id(const char* bdev_name);
void spdk_bdev_ibof_register_io_handler(const char* bdev_name, unvmf_io_handler handler, const char* nqn, uint32_t nqn_id);
#else
void spdk_bdev_ibof_register_io_handler(const char* bdev_name, unvmf_io_handler handler, const char* nqn);
#endif
void spdk_bdev_ibof_unregister_io_handler(const char* bdev_name, unvmf_io_handler handler);
const char* get_attached_subsystem_nqn(const char* bdev_name);

/* uNVMf to ibof volume information */
typedef int (*ibof_bdev_io_handler)(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io);
struct ibof_volume_info {
	uint32_t id;
	char name[VOLUME_NAME_MAX_LEN +1];
	char nqn[SPDK_NVMF_NQN_MAX_LEN+1];
#if defined QOS_ENABLED_FE
        uint32_t nqn_id;
#endif	
	uint64_t size_mb;
	uint64_t iops_limit;
	uint64_t bw_limit;
	/* handler between spdk bdev and ibof bdev */
	ibof_bdev_io_handler ibof_bdev_io;
#if defined NVMe_FLUSH_HANDLING
	/* handler between spdk bdev and ibof bdev for NVMe flush command handling */
	ibof_bdev_io_handler ibof_bdev_flush;
#endif
	/* handler between ibof bdev and unvmf */
	unvmf_io_handler unvmf_io;
};

/* uNVMf to ibof IO descriptor */
typedef void (*ibof_bdev_io_complete_callback)(struct ibof_io* io, int status);
struct ibof_io {
	int ioType;
	uint32_t volume_id;
	struct iovec* iov;
	int iovcnt;
	uint64_t length;
	uint64_t offset;
	void* context;
	ibof_bdev_io_complete_callback complete_cb;
};


#ifdef __cplusplus
}
#endif

#endif /* SPDK_IBOF_H_ */
