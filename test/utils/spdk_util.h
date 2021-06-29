#include "lib/spdk/lib/nvme/nvme_internal.h"

extern struct spdk_nvme_ns* BuildFakeNvmeNamespace(void);
extern void DestroyFakeNvmeNamespace(struct spdk_nvme_ns* ns);
