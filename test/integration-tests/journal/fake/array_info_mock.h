#pragma once

#include <string>

#include "test/integration-tests/journal/utils/test_info.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
class ArrayInfoMock : public IArrayInfo
{
public:
    explicit ArrayInfoMock(TestInfo* _testInfo);
    virtual ~ArrayInfoMock(void);

    virtual const PartitionLogicalSize* GetSizeInfo(PartitionType type);

    // Below are stub functions
    virtual string GetName(void);
    virtual string GetUniqueId(void);
    virtual uint32_t GetIndex(void);
    virtual string GetCreateDatetime(void);
    virtual string GetUpdateDatetime(void);
    virtual string GetMetaRaidType(void);
    virtual string GetDataRaidType(void);
    virtual ArrayStateType GetState(void);
    virtual StateContext* GetStateCtx(void);
    virtual uint32_t GetRebuildingProgress(void);
    virtual bool IsWriteThroughEnabled(void);
    virtual vector<IArrayDevice*> GetDevices(ArrayDeviceType type);

private:
    PartitionLogicalSize* userSizeInfo;
    PartitionLogicalSize* wbSizeInfo;
    TestInfo* testInfo;
};
} // namespace pos
