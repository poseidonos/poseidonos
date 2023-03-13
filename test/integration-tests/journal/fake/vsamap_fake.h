#pragma once

#include <gmock/gmock.h>
#include <mutex>

#include "src/include/address_type.h"
#include "src/mapper/i_vsamap.h"
#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
class VSAMapFake : public IVSAMap
{
public:
    explicit VSAMapFake(TestInfo* testInfo);
    virtual ~VSAMapFake(void);

    MOCK_METHOD(int, SetVSAsInternal, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(int, SetVSAsWithSyncOpen, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    virtual VirtualBlkAddr GetVSAWithSyncOpen(int volId, BlkAddr rba) override;
    virtual VirtualBlkAddr GetVSAInternal(int volumeId, BlkAddr rba, int& caller) override;

    virtual MpageList GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks) override;
    virtual int SetVSAs(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks) { return 0; }
    virtual int GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray) { return 0; }
    virtual VirtualBlkAddr GetRandomVSA(BlkAddr rba) { return UNMAP_VSA; }
    virtual int64_t GetNumUsedBlks(int volId) { return 0; }

private:
    int _SetVSAsInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);
    int _SetVSAsWithSyncOpen(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);

    TestInfo* testInfo;
    VirtualBlkAddr** map;
    std::mutex vsaMapLock;
};
} // namespace pos
