#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/spdk_caller.h"

namespace pos
{
class MockSpdkCaller : public SpdkCaller
{
public:
    using SpdkCaller::SpdkCaller;
    MOCK_METHOD(char*, SpdkNvmfSubsystemGetCtrlrHostnqn, (struct spdk_nvmf_ctrlr * ctrlr), (override));
    MOCK_METHOD(struct spdk_nvmf_ctrlr*, SpdkNvmfSubsystemGetFirstCtrlr, (struct spdk_nvmf_subsystem * subsystem), (override));
    MOCK_METHOD(struct spdk_nvmf_ctrlr*, SpdkNvmfSubsystemGetNextCtrlr, (struct spdk_nvmf_subsystem * subsystem, struct spdk_nvmf_ctrlr* prevCtrlr), (override));
    MOCK_METHOD(int, SpdkNvmfSubsystemSetPauseDirectly, (struct spdk_nvmf_subsystem * subsystem), (override));
    MOCK_METHOD(uint32_t, SpdkNvmfSubsystemGetId, (spdk_nvmf_subsystem * subsystem), (override));
    MOCK_METHOD(struct spdk_bdev*, SpdkBdevCreatePosDisk, (const char* volumeName, uint32_t volumeId,
        const struct spdk_uuid* bdevUuid, uint64_t numBlocks, uint32_t blockSize, bool volumeTypeInMemory,
        const char* arrayName, uint64_t arrayId), (override));
    MOCK_METHOD(void, SpdkBdevDeletePosDisk, (struct spdk_bdev * bdev, pos_bdev_delete_callback cbFunc, void* cbArg), (override));
    MOCK_METHOD(struct spdk_bdev*, SpdkBdevGetByName, (const char* bdevName), (override));
    MOCK_METHOD(const char*, SpdkBdevGetName, (const struct spdk_bdev* bdev), (override));
    MOCK_METHOD(void, SpdkBdevSetQosRateLimits, (struct spdk_bdev * bdev, uint64_t* limits, void (*cbFunc)(void* cbArg, int status), void* cbArg), (override));
    MOCK_METHOD(const char*, SpdkGetAttachedSubsystemNqn, (const char* bdevName), (override));
    MOCK_METHOD(const struct spdk_uuid*, SpdkBdevGetUuid, (const struct spdk_bdev* bdev), (override));
    MOCK_METHOD(int, SpdkUuidFmtLower, (char* uuid_str, size_t uuid_str_size, const struct spdk_uuid* uuid), (override));
    MOCK_METHOD(int, SpdkUuidParse, (struct spdk_uuid * uuid, const char* uuid_str), (override));
};

} // namespace pos
