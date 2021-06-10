#pragma once
#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/nvmf.h"
#include "spdk/pos.h"
#include "src/lib/singleton.h"

namespace pos
{
class SpdkCaller
{
public:
    SpdkCaller(void);
    ~SpdkCaller(void);
    struct spdk_nvmf_tgt* SpdkNvmfGetTgt(const char* name);
    struct spdk_nvmf_subsystem* SpdkNvmfTgtFindSubsystem(struct spdk_nvmf_tgt* tgt, const char* subnqn);
    enum spdk_nvmf_subtype SpdkNvmfSubsystemGetType(struct spdk_nvmf_subsystem* subsystem);
    const char* SpdkNvmfSubsystemGetNqn(const struct spdk_nvmf_subsystem* subsystem);
    struct spdk_nvmf_subsystem* SpdkNvmfSubsystemGetFirst(struct spdk_nvmf_tgt* tgt);
    struct spdk_nvmf_subsystem* SpdkNvmfSubsystemGetNext(struct spdk_nvmf_subsystem* subsystem);
    char* SpdkNvmfSubsystemGetCtrlrHostnqn(struct spdk_nvmf_ctrlr* ctrlr);
    struct spdk_nvmf_ctrlr* SpdkNvmfSubsystemGetFirstCtrlr(struct spdk_nvmf_subsystem* subsystem);
    struct spdk_nvmf_ctrlr* SpdkNvmfSubsystemGetNextCtrlr(struct spdk_nvmf_subsystem* subsystem,
        struct spdk_nvmf_ctrlr* prev_ctrlr);
    struct spdk_nvmf_ns* SpdkNvmfSubsystemGetFirstNs(struct spdk_nvmf_subsystem* subsystem);
    struct spdk_nvmf_ns* SpdkNvmfSubsystemGetNextNs(struct spdk_nvmf_subsystem* subsystem,
        struct spdk_nvmf_ns* prev_ns);
    int SpdkNvmfSubsystemPause(struct spdk_nvmf_subsystem* subsystem, spdk_nvmf_subsystem_state_change_done cb_fn, void* cb_arg);
    int SpdkNvmfSubsystemResume(struct spdk_nvmf_subsystem* subsystem, spdk_nvmf_subsystem_state_change_done cb_fn, void* cb_arg);
    struct spdk_bdev* SpdkNvmfNsGetBdev(struct spdk_nvmf_ns* ns);
    uint32_t SpdkNvmfNsGetId(const struct spdk_nvmf_ns* ns);
    uint32_t SpdkNvmfSubsystemGetId(spdk_nvmf_subsystem* subsystem);
    struct spdk_bdev* SpdkBdevCreatePosDisk(const char* volume_name, uint32_t volume_id,
        const struct spdk_uuid* bdev_uuid, uint64_t num_blocks, uint32_t block_size,
        bool volume_type_in_memory, const char* array_name, uint64_t array_id);
    void SpdkBdevDeletePosDisk(struct spdk_bdev* bdev, pos_bdev_delete_callback cb_fn, void* cb_arg);
    struct spdk_bdev* SpdkBdevGetByName(const char* bdev_name);
    const char* SpdkBdevGetName(const struct spdk_bdev* bdev);
    void SpdkBdevSetQosRateLimits(struct spdk_bdev* bdev, uint64_t* limits,
        void (*cb_fn)(void* cb_arg, int status), void* cb_arg);
};

using SpdkCallerSingleton = Singleton<SpdkCaller>;
} // namespace pos
