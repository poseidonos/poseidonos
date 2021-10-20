#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_context.h"

namespace pos
{
class MockQosContext : public QosContext
{
public:
    using QosContext::QosContext;
    MOCK_METHOD(void, InsertActiveVolume, (uint32_t), (override));
    MOCK_METHOD(bool, GetVolumeOperationDone, (), (override));
    MOCK_METHOD(bool, GetApplyCorrection, (), (override));
    MOCK_METHOD(bool, AllReactorsProcessed, (), (override));
    MOCK_METHOD(QosUserPolicy&, GetQosUserPolicy, (), (override));
    MOCK_METHOD(QosParameters&, GetQosParameters, (), (override));
    MOCK_METHOD(bool, IsCorrectionCycleOver, (), (override));
    /*MOCK_METHOD(QosCorrection, GetQosCorrection, (), (override));
    MOCK_METHOD(void, SetApplyCorrection, (), (override));
    MOCK_METHOD(std::map<uint32_t, map<uint32_t, uint32_t>>, GetActiveVolumeReactors, (), (override));*/
};

} // namespace pos
