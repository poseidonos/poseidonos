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

#include "bdev_pos.h"
#include "spdk/rpc.h"
#include "spdk/util.h"
#include "spdk/uuid.h"
#include "spdk/string.h"
#include "spdk/log.h"

struct rpc_construct_pos {
	char *name;
	char *uuid;
	char *array_name;
	uint32_t volume_id;
	uint64_t volume_size_mb;
	uint32_t volume_type_in_memory;
};

static void
free_rpc_construct_pos(struct rpc_construct_pos *r)
{
	if (r->name) { free(r->name); }
	if (r->uuid) { free(r->uuid); }
	if (r->array_name) { free(r->array_name); }
}

static const struct spdk_json_object_decoder rpc_construct_pos_decoders[] = {
	{"name", offsetof(struct rpc_construct_pos, name), spdk_json_decode_string, true},
	{"volume_id", offsetof(struct rpc_construct_pos, volume_id), spdk_json_decode_uint32},
	{"uuid", offsetof(struct rpc_construct_pos, uuid), spdk_json_decode_string, true},
	{"volume_size_mb", offsetof(struct rpc_construct_pos, volume_size_mb), spdk_json_decode_uint64},
	{"volume_type_in_memory", offsetof(struct rpc_construct_pos, volume_type_in_memory), spdk_json_decode_uint32},
	{"array_name", offsetof(struct rpc_construct_pos, array_name), spdk_json_decode_string, true},
};

static void
spdk_rpc_bdev_pos_create(struct spdk_jsonrpc_request *request,
			  const struct spdk_json_val *params)
{
	struct rpc_construct_pos req = {NULL};
	struct spdk_json_write_ctx *w;
	struct spdk_uuid *uuid = NULL;
	struct spdk_uuid decoded_uuid;
	struct spdk_bdev *bdev;
	uint32_t block_size = 512;
	uint32_t volume_id = 0;
	uint64_t volume_size_mb = (1024 * 1024);
	bool volume_type_in_memory = false;

	if (spdk_json_decode_object(params, rpc_construct_pos_decoders,
				    SPDK_COUNTOF(rpc_construct_pos_decoders),
				    &req)) {
		SPDK_DEBUGLOG(bdev_pos, "spdk_json_decode_object failed\n");
		goto invalid;
	}

	if (req.uuid) {
		if (spdk_uuid_parse(&decoded_uuid, req.uuid)) {
			goto invalid;
		}
		uuid = &decoded_uuid;
	}

	volume_id = req.volume_id;
	volume_size_mb *= req.volume_size_mb;
	volume_type_in_memory = (req.volume_type_in_memory == 0) ? false : true;
	bdev = create_pos_disk(req.name, volume_id, uuid, volume_size_mb / block_size, block_size,
				volume_type_in_memory, req.array_name);
	if (bdev == NULL) {
		SPDK_ERRLOG("Could not create pos disk\n");
		goto invalid;
	}

	free_rpc_construct_pos(&req);

	w = spdk_jsonrpc_begin_result(request);
	if (w == NULL) {
		goto invalid;
	}

	spdk_json_write_string(w, spdk_bdev_get_name(bdev));
	spdk_jsonrpc_end_result(request, w);
	return;

invalid:
	free_rpc_construct_pos(&req);
	spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS, "Invalid parameters");
}
SPDK_RPC_REGISTER("bdev_pos_create", spdk_rpc_bdev_pos_create, SPDK_RPC_RUNTIME)
SPDK_RPC_REGISTER_ALIAS_DEPRECATED(bdev_pos_create, construct_pos_bdev)

struct rpc_delete_pos {
	char *name;
};

static void
free_rpc_delete_pos(struct rpc_delete_pos *r)
{
	if (r->name) { free(r->name); }
}

static const struct spdk_json_object_decoder rpc_delete_pos_decoders[] = {
	{"name", offsetof(struct rpc_delete_pos, name), spdk_json_decode_string},
};

static void
_spdk_rpc_bdev_pos_delete_cb(void *cb_arg, int bdeverrno)
{
	struct spdk_jsonrpc_request *request = cb_arg;
	struct spdk_json_write_ctx *w;

	w = spdk_jsonrpc_begin_result(request);
	if (w == NULL) {
		return;
	}

	spdk_json_write_bool(w, bdeverrno == 0);
	spdk_jsonrpc_end_result(request, w);
}

static void
spdk_rpc_bdev_pos_delete(struct spdk_jsonrpc_request *request,
			  const struct spdk_json_val *params)
{
	int rc;
	struct rpc_delete_pos req = {NULL};
	struct spdk_bdev *bdev;

	if (spdk_json_decode_object(params, rpc_delete_pos_decoders,
				    SPDK_COUNTOF(rpc_delete_pos_decoders),
				    &req)) {
		SPDK_DEBUGLOG(bdev_pos, "spdk_json_decode_object failed\n");
		rc = -EINVAL;
		goto invalid;
	}

	bdev = spdk_bdev_get_by_name(req.name);
	if (bdev == NULL) {
		SPDK_INFOLOG(bdev_pos, "bdev '%s' does not exist\n", req.name);
		rc = -ENODEV;
		goto invalid;
	}

	delete_pos_disk(bdev, _spdk_rpc_bdev_pos_delete_cb, request);

	free_rpc_delete_pos(&req);

	return;

invalid:
	free_rpc_delete_pos(&req);
	spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS, spdk_strerror(-rc));
}
SPDK_RPC_REGISTER("bdev_pos_delete", spdk_rpc_bdev_pos_delete, SPDK_RPC_RUNTIME)
SPDK_RPC_REGISTER_ALIAS_DEPRECATED(bdev_pos_delete, delete_pos_bdev)
