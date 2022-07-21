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

#include "test/unit-tests/utils/spdk_util.h"


struct spdk_nvme_ns* BuildFakeNvmeNamespace(void)
{
    // Initialize spdk_nvme_ctrlr's proc queue and mutex
    struct spdk_nvme_ctrlr* fakeController = new struct spdk_nvme_ctrlr;
    fakeController->trid.trtype = SPDK_NVME_TRANSPORT_PCIE;
    TAILQ_INIT(&fakeController->active_procs);
    pthread_mutex_init(&fakeController->ctrlr_lock, NULL);

    // Initialize spdk_nvme_ns's reference to controller
    struct spdk_nvme_ns* fakeNamespace = new spdk_nvme_ns;
    fakeNamespace->ctrlr = fakeController;
    struct spdk_pci_device devhandle;
    devhandle.socket_id = 1;

    struct spdk_nvme_ctrlr_process* ctrlr_proc = static_cast<spdk_nvme_ctrlr_process*>(
        malloc(sizeof(struct spdk_nvme_ctrlr_process)));
    {
        ctrlr_proc->is_primary = false;
        // the following value is compared deep inside at nvme_ctrlr_get_current_process();
        ctrlr_proc->pid = getpid();
        STAILQ_INIT(&ctrlr_proc->active_reqs);
        ctrlr_proc->devhandle = &devhandle;
        ctrlr_proc->ref = 0;
        TAILQ_INIT(&ctrlr_proc->allocated_io_qpairs);
        TAILQ_INSERT_TAIL(&fakeController->active_procs, ctrlr_proc, tailq);

        // test myself
        struct spdk_nvme_ctrlr_process *active_proc;
        TAILQ_FOREACH(active_proc, &fakeController->active_procs, tailq)
        {
            if (active_proc->pid == getpid())
            {
                // since there's only one entry at this moment
            }
            else
            {
                // SPDK internals might have changed! Returning NULL for error propagation.
                return NULL;
            }
        }
    }
    return fakeNamespace;
}

void DestroyFakeNvmeNamespace(struct spdk_nvme_ns* ns)
{
    struct spdk_nvme_ctrlr* ctrl = ns->ctrlr;
    delete ns;
    delete ctrl;
}
