#include "src/qos/qos_event_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/bio/ubio.h"
#include "src/include/backend_event.h"
#include "src/qos/rate_limit.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/submission_adapter_mock.h"
#include "test/unit-tests/qos/submission_notifier_mock.h"
#include "src/include/memory.h"
using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosEventManager, QosEventManager_Constructor_One_Stack)
{
    QosEventManager qosEventManager();
}
TEST(QosEventManager, QosEventManager_Constructor_One_Heap)
{
    QosEventManager* qosEventManager = new QosEventManager();
    delete qosEventManager;
}
TEST(QosEventManager, HandleEventUbioSubmission_Test)
{
    QosEventManager qosEventManager;
    NiceMock<MockSubmissionNotifier> mockSubmissionNotifier;
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);
    uint32_t id = 1;
    NiceMock<MockSubmissionAdapter> mockSubmissionAdapter;
    qosEventManager.HandleEventUbioSubmission(&mockSubmissionAdapter, &mockSubmissionNotifier, id, ubio);
}

TEST(QosEventManager, HandleEventUbioSubmission_Test_RateLimitTrue)
{
    QosEventManager qosEventManager;
    NiceMock<MockSubmissionNotifier> mockSubmissionNotifier;
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, DEFAULT_MAX_BW_IOPS+1, 0);
    uint32_t id = 1;
    ubio->SetEventType(BackendEvent_Flush);
    NiceMock<MockSubmissionAdapter> mockSubmissionAdapter;
    qosEventManager.HandleEventUbioSubmission(&mockSubmissionAdapter, &mockSubmissionNotifier, id, ubio);
    int buf2[10];
    UbioSmartPtr ubio2 = std::make_shared<Ubio>((void*)buf2, DEFAULT_MAX_BW_IOPS, 0);
    ubio2->SetEventType(BackendEvent_Flush);
    qosEventManager.HandleEventUbioSubmission(&mockSubmissionAdapter, &mockSubmissionNotifier, id, ubio2);
}

TEST(QosEventManager, HandleEventUbioSubmission_BackendVenetFlush_Test)
{
    QosEventManager qosEventManager;
    NiceMock<MockSubmissionNotifier> mockSubmissionNotifier;
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);
    ubio->SetEventType(BackendEvent_Flush);

    uint32_t id = 1;
    NiceMock<MockSubmissionAdapter> mockSubmissionAdapter;
    qosEventManager.HandleEventUbioSubmission(&mockSubmissionAdapter, &mockSubmissionNotifier, id, ubio);
}

TEST(QosEventManager, Check_IOWorkerPoller_Dequeue_Params)
{
    QosEventManager qosEventManager;
    uint32_t id = 1;
    NiceMock<MockSubmissionAdapter> mockSubmissionAdapter;
    uint32_t retVal = qosEventManager.IOWorkerPoller(id, &mockSubmissionAdapter);
    ASSERT_GE(retVal, 0);
    BackendEvent eventId = BackendEvent_Flush;
    bw_iops_parameter params = qosEventManager.DequeueParams(id, eventId);
    ASSERT_NE(&params, NULL);
}

TEST(QosEventManager, Check_Setter_Getter_eventWeightWRR)
{
    QosEventManager qosEventManager;
    BackendEvent eventId = BackendEvent_Flush;
    int64_t weight = 100;
    qosEventManager.SetEventWeightWRR(eventId, weight);
    int64_t returnedWeight = qosEventManager.GetEventWeightWRR(eventId);
    ASSERT_EQ(returnedWeight, weight);
}

} // namespace pos
