#pragma once

#include "gmock/gmock.h"

#include "src/include/address_type.h"
#include "src/mapper/i_vsamap.h"
#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
class VSAMapMock : public IVSAMap
{
public:
    explicit VSAMapMock(TestInfo* testInfo);
    virtual ~VSAMapMock(void);

    virtual VirtualBlkAddr GetVSAInternal(int volumeId, BlkAddr rba,
        int& caller) override;
    virtual MpageList GetDirtyVsaMapPages(int volumeId, BlkAddr startRba,
        uint64_t numBlks) override;

    MOCK_METHOD(int, SetVSAsInternal, (int volumeId, BlkAddr startRba,
        VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(int, SetVSAsWithSyncOpen, (int volumeId, BlkAddr startRba,
        VirtualBlks& virtualBlks), (override));

    virtual int GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks,
        VsaArray& vsaArray) override;
    virtual int SetVSAs(int volumeId, BlkAddr startRba,
        VirtualBlks& virtualBlks) override;
    virtual VirtualBlkAddr GetRandomVSA(BlkAddr rba) override;
    virtual int64_t GetNumUsedBlks(int volId) override;
    virtual VirtualBlkAddr GetVSAWithSyncOpen(int volId, BlkAddr rba) override;
    //virtual int SetVSAsWithSyncOpen(int volId, BlkAddr startRba, VirtualBlks& virtualBlks) override;

private:
    int _SetVSAsInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);
    int _SetVSAsWithSyncOpen(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);

    TestInfo* testInfo;
    VirtualBlkAddr** map;
};
} // namespace pos
