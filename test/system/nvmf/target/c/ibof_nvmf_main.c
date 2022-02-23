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

#include "spdk/stdinc.h"

#include "spdk/env.h"
#include "spdk/event.h"

static void
nvmf_usage(void)
{
}

static int
nvmf_parse_arg(int ch, char *arg)
{
	return 0;
}

static void
nvmf_tgt_started(void *arg1)
{
	if (getenv("MEMZONE_DUMP") != NULL) {
		spdk_memzone_dump(stdout);
		fflush(stdout);
	}

	spdk_log_set_level(SPDK_LOG_WARN);
	spdk_log_set_print_level(SPDK_LOG_WARN);
	spdk_log_clear_flag("reactor");

	/* tracepoints nvmf <-> nvme 
	spdk_log_set_flag("nvmf");
	spdk_log_set_flag("rdma");
	spdk_log_set_flag("bdev");
	spdk_log_set_flag("bdev_nvme");
	spdk_log_set_flag("nvme");
	spdk_log_set_flag("bdev_malloc");
	spdk_log_set_flag("bdev_ibof");
	*/

	printf("debug level changed\n");
		
}

int
main(int argc, char **argv)
{
	int rc;
	struct spdk_app_opts opts = {};

	/* default value in opts */
	spdk_app_opts_init(&opts);
	opts.name = "ibof_nvmf";
	//opts.max_delay_us = 1000; /* 1 ms */
	if ((rc = spdk_app_parse_args(argc, argv, &opts, "", NULL,
				      nvmf_parse_arg, nvmf_usage)) !=
	    SPDK_APP_PARSE_ARGS_SUCCESS) {
		exit(rc);
	}

	/* Blocks until the application is exiting */
	rc = spdk_app_start(&opts, nvmf_tgt_started, NULL);
	spdk_app_fini();
	return rc;
}
