#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_manager.h"

namespace pos
{
class MockQosManager : public QosManager
{
public:
    using QosManager::QosManager;
    MOCK_METHOD(int64_t, GetEventWeightWRR, (BackendEvent event), (override));
    MOCK_METHOD(int64_t, GetNoContentionCycles, (), ());
    MOCK_METHOD(bool, IsFeQosEnabled, (), (override));
    MOCK_METHOD(void, _Finalize, (), (override));
    MOCK_METHOD(void, DecreasePendingBackendEvents, (BackendEvent event), (override));
    MOCK_METHOD(void, IncreasePendingBackendEvents, (BackendEvent event), (override));
    MOCK_METHOD(void, LogEvent, (BackendEvent event), (override));
    MOCK_METHOD(int, IOWorkerPoller, (uint32_t id, SubmissionAdapter* ioSubmission), (override));
    MOCK_METHOD(void, HandleEventUbioSubmission, (SubmissionAdapter* ioSubmission,
        SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio), (override));
    MOCK_METHOD(bw_iops_parameter, DequeueEventParams, (uint32_t workerId, BackendEvent eventId), (override));
    MOCK_METHOD(bool, IsMinimumPolicyInEffectInSystem, (), (override));
};
} // namespace pos
