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
    virtual ~SpdkCaller(void);
    virtual struct spdk_nvmf_tgt* SpdkNvmfGetTgt(const char* name);
    virtual struct spdk_nvmf_subsystem* SpdkNvmfTgtFindSubsystem(struct spdk_nvmf_tgt* tgt, const char* subnqn);
    virtual enum spdk_nvmf_subtype SpdkNvmfSubsystemGetType(struct spdk_nvmf_subsystem* subsystem);
    virtual const char* SpdkNvmfSubsystemGetNqn(const struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_subsystem* SpdkNvmfSubsystemGetFirst(struct spdk_nvmf_tgt* tgt);
    virtual struct spdk_nvmf_subsystem* SpdkNvmfSubsystemGetNext(struct spdk_nvmf_subsystem* subsystem);
    virtual char* SpdkNvmfSubsystemGetCtrlrHostnqn(struct spdk_nvmf_ctrlr* ctrlr);
    virtual struct spdk_nvmf_ctrlr* SpdkNvmfSubsystemGetFirstCtrlr(struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_ctrlr* SpdkNvmfSubsystemGetNextCtrlr(struct spdk_nvmf_subsystem* subsystem,
        struct spdk_nvmf_ctrlr* prevCtrlr);
    virtual struct spdk_nvmf_ns* SpdkNvmfSubsystemGetFirstNs(struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_ns* SpdkNvmfSubsystemGetNextNs(struct spdk_nvmf_subsystem* subsystem,
        struct spdk_nvmf_ns* prevNs);
    virtual int SpdkNvmfSubsystemPause(struct spdk_nvmf_subsystem* subsystem, spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg);
    virtual int SpdkNvmfSubsystemResume(struct spdk_nvmf_subsystem* subsystem, spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg);
    virtual struct spdk_bdev* SpdkNvmfNsGetBdev(struct spdk_nvmf_ns* ns);
    virtual uint32_t SpdkNvmfNsGetId(const struct spdk_nvmf_ns* ns);
    virtual uint32_t SpdkNvmfSubsystemGetId(spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_bdev* SpdkBdevCreatePosDisk(const char* volumeName, uint32_t volumeId,
        const struct spdk_uuid* bdevUuid, uint64_t numBlocks, uint32_t blockSize,
        bool volumeTypeInMemory, const char* arrayName, uint64_t arrayId);
    virtual void SpdkBdevDeletePosDisk(struct spdk_bdev* bdev, pos_bdev_delete_callback cbFunc, void* cbArg);
    virtual struct spdk_bdev* SpdkBdevGetByName(const char* bdevName);
    virtual const char* SpdkBdevGetName(const struct spdk_bdev* bdev);
    virtual void SpdkBdevSetQosRateLimits(struct spdk_bdev* bdev, uint64_t* limits,
        void (*cbFunc)(void* cbArg, int status), void* cbArg);
    virtual const char* SpdkGetAttachedSubsystemNqn(const char* bdevName);
};

using SpdkCallerSingleton = Singleton<SpdkCaller>;
} // namespace pos
