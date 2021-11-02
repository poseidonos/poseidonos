#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/caller/spdk_nvmf_caller.h"

namespace pos
{
class MockSpdkNvmfCaller : public SpdkNvmfCaller
{
public:
    using SpdkNvmfCaller::SpdkNvmfCaller;
    MOCK_METHOD(struct spdk_nvmf_tgt*, SpdkNvmfGetTgt, (const char* name), (override));
    MOCK_METHOD(struct spdk_nvmf_subsystem*, SpdkNvmfTgtFindSubsystem, (struct spdk_nvmf_tgt * tgt, const char* subnqn), (override));
    MOCK_METHOD(enum spdk_nvmf_subtype, SpdkNvmfSubsystemGetType, (struct spdk_nvmf_subsystem * subsystem), (override));
    MOCK_METHOD(const char*, SpdkNvmfSubsystemGetNqn, (const struct spdk_nvmf_subsystem* subsystem), (override));
    MOCK_METHOD(struct spdk_nvmf_subsystem*, SpdkNvmfSubsystemGetFirst, (struct spdk_nvmf_tgt * tgt), (override));
    MOCK_METHOD(struct spdk_nvmf_subsystem*, SpdkNvmfSubsystemGetNext, (struct spdk_nvmf_subsystem * subsystem), (override));
    MOCK_METHOD(struct spdk_nvmf_ns*, SpdkNvmfSubsystemGetFirstNs, (struct spdk_nvmf_subsystem * subsystem), (override));
    MOCK_METHOD(struct spdk_nvmf_ns*, SpdkNvmfSubsystemGetNextNs, (struct spdk_nvmf_subsystem * subsystem, struct spdk_nvmf_ns* prevNs), (override));
    MOCK_METHOD(int, SpdkNvmfSubsystemPause, (struct spdk_nvmf_subsystem * subsystem, uint32_t nsid, spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg), (override));
    MOCK_METHOD(int, SpdkNvmfSubsystemResume, (struct spdk_nvmf_subsystem * subsystem, spdk_nvmf_subsystem_state_change_done cbFunc, void* cbArg), (override));
    MOCK_METHOD(struct spdk_bdev*, SpdkNvmfNsGetBdev, (struct spdk_nvmf_ns * ns), (override));
    MOCK_METHOD(uint32_t, SpdkNvmfNsGetId, (const struct spdk_nvmf_ns* ns), (override));
    MOCK_METHOD(void, SpdkNvmfInitializeNumaAwarePollGroup, (), (override));
};

} // namespace pos
