#pragma once

#include <string>

#include "test/integration-tests/journal/utils/test_info.h"
#include "src/array_models/interface/i_array_info.h"
#ifdef _ADMIN_ENABLED
#include "src/array/device/i_array_device_manager.h"
#endif

namespace pos
{
class ArrayInfoMock : public IArrayInfo
{
public:
    explicit ArrayInfoMock(TestInfo* _testInfo);
    virtual ~ArrayInfoMock(void);

    virtual const PartitionLogicalSize* GetSizeInfo(PartitionType type);

    // Below are stub functions
    virtual DeviceSet<string> GetDevNames(void);
    virtual string GetName(void);
    virtual unsigned int GetIndex(void);
    virtual string GetMetaRaidType(void);
    virtual string GetDataRaidType(void);
    virtual id_t GetUniqueId(void);
    virtual ArrayStateType GetState(void);
    virtual StateContext* GetStateCtx(void);
    virtual uint32_t GetRebuildingProgress(void);
    virtual string GetCreateDatetime(void);
    virtual string GetUpdateDatetime(void);
    virtual bool IsWriteThroughEnabled(void);
#ifdef _ADMIN_ENABLED
    virtual IArrayDevMgr*
    GetArrayManager(void)
    {
        return nullptr;
    }
#endif

private:
    PartitionLogicalSize* userSizeInfo;
    TestInfo* testInfo;
};
} // namespace pos
