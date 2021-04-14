#include "test/utils/spdk_util.h"

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
    fakeController->ns = fakeNamespace;

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
