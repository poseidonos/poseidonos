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

#ifndef SPDK_POS_NVMF_H_
#define SPDK_POS_NVMF_H_

#include "spdk/stdinc.h"
#include "nvmf_spec.h"
#include "pos.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NR_MAX_NAMESPACE 128
#define NR_MAX_TRANSPORT 4

typedef void (*pos_bdev_delete_callback)(void *cb_arg, int bdeverrno);

/* uNVMf composition descriptor */
struct nvmf_namespace {
	char name[32];
	uint16_t nsid;
};

struct nvmf_transport {
	char type[16];
	char adrfam[16];
	char traddr[SPDK_NVMF_TRADDR_MAX_LEN + 1];
	char trsvcid[SPDK_NVMF_TRSVCID_MAX_LEN + 1];
};

struct nvmf_subsystem {
	char nqn[SPDK_NVMF_NQN_MAX_LEN + 1];
	uint16_t nr_namespace;
	struct nvmf_namespace ns[NR_MAX_NAMESPACE];
	uint16_t nr_transport;
	struct nvmf_transport tr[NR_MAX_TRANSPORT];
};

/*
 * create pos_bdev disk that will be attached on uNVMf
 */
struct spdk_bdev *spdk_bdev_create_pos_disk(const char *volume_name, uint32_t volume_id,
		const struct spdk_uuid *bdev_uuid, uint64_t num_blocks, uint32_t block_size,
		bool volume_type_in_memory, const char *array_name);

/*
 * delete pos_bdev disk
 */
void spdk_bdev_delete_pos_disk(struct spdk_bdev *bdev, pos_bdev_delete_callback cb_fn,
				void *cb_arg);
#ifdef __cplusplus
}
#endif

#endif /* SPDK_POS_H_ */
