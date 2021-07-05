#include "src/spdk_wrapper/spdk_caller.h"

#include "spdk/bdev.h"
#include "spdk/bdev_module.h"

namespace pos
{
SpdkCaller::SpdkCaller(void)
{
}

SpdkCaller::~SpdkCaller(void)
{
}

struct spdk_nvmf_tgt*
SpdkCaller::SpdkNvmfGetTgt(const char* name)
{
    return spdk_nvmf_get_tgt(name);
}

struct spdk_nvmf_subsystem*
SpdkCaller::SpdkNvmfTgtFindSubsystem(struct spdk_nvmf_tgt* tgt, const char* subnqn)
{
    return spdk_nvmf_tgt_find_subsystem(tgt, subnqn);
}

enum spdk_nvmf_subtype
SpdkCaller::SpdkNvmfSubsystemGetType(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_type(subsystem);
}

const char*
SpdkCaller::SpdkNvmfSubsystemGetNqn(const struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_nqn(subsystem);
}

struct spdk_nvmf_subsystem*
SpdkCaller::SpdkNvmfSubsystemGetFirst(struct spdk_nvmf_tgt* tgt)
{
    return spdk_nvmf_subsystem_get_first(tgt);
}

struct spdk_nvmf_subsystem*
SpdkCaller::SpdkNvmfSubsystemGetNext(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_next(subsystem);
}

char*
SpdkCaller::SpdkNvmfSubsystemGetCtrlrHostnqn(struct spdk_nvmf_ctrlr* ctrlr)
{
    return spdk_nvmf_subsystem_get_ctrlr_hostnqn(ctrlr);
}

struct spdk_nvmf_ctrlr*
SpdkCaller::SpdkNvmfSubsystemGetFirstCtrlr(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_first_ctrlr(subsystem);
}

struct spdk_nvmf_ctrlr*
SpdkCaller::SpdkNvmfSubsystemGetNextCtrlr(struct spdk_nvmf_subsystem* subsystem,
    struct spdk_nvmf_ctrlr* prevCtrlr)
{
    return spdk_nvmf_subsystem_get_next_ctrlr(subsystem, prevCtrlr);
}

struct spdk_nvmf_ns*
SpdkCaller::SpdkNvmfSubsystemGetFirstNs(struct spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_first_ns(subsystem);
}

struct spdk_nvmf_ns*
SpdkCaller::SpdkNvmfSubsystemGetNextNs(struct spdk_nvmf_subsystem* subsystem,
    struct spdk_nvmf_ns* prevNs)
{
    return spdk_nvmf_subsystem_get_next_ns(subsystem, prevNs);
}

int
SpdkCaller::SpdkNvmfSubsystemPause(struct spdk_nvmf_subsystem* subsystem,
    spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg)
{
    return spdk_nvmf_subsystem_pause(subsystem, cbFunc, cbArg);
}

int
SpdkCaller::SpdkNvmfSubsystemResume(struct spdk_nvmf_subsystem* subsystem,
    spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg)
{
    return spdk_nvmf_subsystem_resume(subsystem, cbFunc, cbArg);
}

struct spdk_bdev*
SpdkCaller::SpdkNvmfNsGetBdev(struct spdk_nvmf_ns* ns)
{
    return spdk_nvmf_ns_get_bdev(ns);
}

uint32_t
SpdkCaller::SpdkNvmfNsGetId(const struct spdk_nvmf_ns* ns)
{
    return spdk_nvmf_ns_get_id(ns);
}

uint32_t
SpdkCaller::SpdkNvmfSubsystemGetId(spdk_nvmf_subsystem* subsystem)
{
    return spdk_nvmf_subsystem_get_id(subsystem);
}

struct spdk_bdev*
SpdkCaller::SpdkBdevCreatePosDisk(const char* volume_name, uint32_t volumeId,
    const struct spdk_uuid* bdevUuid, uint64_t numBlocks, uint32_t blockSize,
    bool volume_type_in_memory, const char* array_name, uint64_t arrayId)
{
    return spdk_bdev_create_pos_disk(volume_name, volumeId, bdevUuid, numBlocks, blockSize,
        volume_type_in_memory, array_name, arrayId);
}

void
SpdkCaller::SpdkBdevDeletePosDisk(struct spdk_bdev* bdev, pos_bdev_delete_callback cbFunc, void* cbArg)
{
    return spdk_bdev_delete_pos_disk(bdev, cbFunc, cbArg);
}

struct spdk_bdev*
SpdkCaller::SpdkBdevGetByName(const char* bdevName)
{
    return spdk_bdev_get_by_name(bdevName);
}

const char*
SpdkCaller::SpdkBdevGetName(const struct spdk_bdev* bdev)
{
    return spdk_bdev_get_name(bdev);
}

void
SpdkCaller::SpdkBdevSetQosRateLimits(struct spdk_bdev* bdev, uint64_t* limits,
    void (*cbFunc)(void* cbArg, int status), void* cbArg)
{
    return spdk_bdev_set_qos_rate_limits(bdev, limits, cbFunc, cbArg);
}

const char*
SpdkCaller::SpdkGetAttachedSubsystemNqn(const char* bdevName)
{
    return get_attached_subsystem_nqn(bdevName);
}
} // namespace pos
