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

#ifndef SPDK_POS_VOLUME_H_
#define SPDK_POS_VOLUME_H_

#include "spdk/stdinc.h"
#include "nvmf_spec.h"
#include "pos.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POS_IO_STATUS_SUCCESS (0)
#define POS_IO_STATUS_FAIL (-1)

#define VOLUME_NAME_MAX_LEN (255)
#define NR_MAX_VOLUME (256)
#define ARRAY_NAME_MAX_LEN (63)

enum IO_TYPE {
	READ = 0,
	WRITE,
	FLUSH
#ifdef _ADMIN_ENABLED
	,
	ADMIN = 100,
	GET_LOG_PAGE
#endif
};
/*
 * register the IO submit/compete callback that links uNVMf pos_bdev and Frontend
 */
typedef int (*unvmf_submit_handler)(struct pos_io *io);
typedef void (*unvmf_complete_handler)(void);
typedef struct unvmf_io_handler {
	unvmf_submit_handler submit;
	unvmf_complete_handler complete;
} unvmf_io_handler;
uint32_t get_attached_subsystem_id(const char *bdev_name);
void spdk_bdev_pos_register_io_handler(const char *bdev_name, unvmf_io_handler handler);

struct spdk_thread *get_nvmf_thread_from_reactor(int reactor);
void spdk_bdev_pos_unregister_io_handler(const char *bdev_name);

void set_pos_volume_info(const char *bdev_name, const char *nqn, int nqn_id);
void reset_pos_volume_info(const char *bdev_name);
void send_msg_to_all_nvmf_thread(uint32_t current_core, void *fn, void *arg1);
const char *get_attached_subsystem_nqn(const char *bdev_name);

/* uNVMf to pos volume information */
typedef int (*pos_bdev_io_handler)(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io);
struct pos_volume_info {
	uint32_t id;
	char name[VOLUME_NAME_MAX_LEN + 1];
	char nqn[SPDK_NVMF_NQN_MAX_LEN + 1];
	char array_name[ARRAY_NAME_MAX_LEN + 1];
	uint32_t nqn_id;
	uint64_t size_mb;
	uint64_t iops_limit;
	uint64_t bw_limit;
	/* handler between spdk bdev and pos bdev */
	pos_bdev_io_handler pos_bdev_io;
	/* handler between spdk bdev and pos bdev for NVMe flush command handling */
	pos_bdev_io_handler pos_bdev_flush;
#ifdef _ADMIN_ENABLED
	/* handler between spdk bdev and pos bdev for admin commands */
	pos_bdev_io_handler pos_bdev_admin;
#endif

	/* handler between pos bdev and unvmf */
	unvmf_io_handler unvmf_io;
};

/* uNVMf to pos IO descriptor */
typedef void (*pos_bdev_io_complete_callback)(struct pos_io *io, int status);
struct pos_io {
	int ioType;
	uint32_t volume_id;
	struct iovec *iov;
	int iovcnt;
	uint64_t length;
	uint64_t offset;
	void *context;
	char *arrayName;
	pos_bdev_io_complete_callback complete_cb;
};


#ifdef __cplusplus
}
#endif

#endif /* SPDK_POS_H_ */
