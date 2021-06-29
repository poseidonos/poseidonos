#include "src/qos/qos_manager.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(QosManager, QosManager_)
{
}

TEST(QosManager, Initialize_)
{
}

TEST(QosManager, EventQosPoller_)
{
}

TEST(QosManager, SubmitAsyncIO_)
{
}

TEST(QosManager, UpdateVolumePolicy_)
{
}

TEST(QosManager, GetVolumePolicy_)
{
}

TEST(QosManager, DequeueVolumeParams_)
{
}

TEST(QosManager, DequeueEventParams_)
{
}

TEST(QosManager, SetVolumeWeight_)
{
}

TEST(QosManager, SetEventWeight_)
{
}

TEST(QosManager, SetEventWeightWRR_)
{
}

TEST(QosManager, GetEventWeightWRR_)
{
}

TEST(QosManager, GetVolumeWeight_)
{
}

TEST(QosManager, GetEventWeight_)
{
}

TEST(QosManager, GetUsedStripeCnt_)
{
}

TEST(QosManager, IncreaseUsedStripeCnt_)
{
}

TEST(QosManager, DecreaseUsedStripeCnt_)
{
}

TEST(QosManager, UpdateSubsystemToVolumeMap_)
{
}

TEST(QosManager, GetRemainingUserRebuildSegmentCnt_)
{
}

TEST(QosManager, GetRemainingMetaRebuildStripeCnt_)
{
}

TEST(QosManager, GetFreeSegmentCnt_)
{
}

TEST(QosManager, IncreasePendingEvents_)
{
}

TEST(QosManager, DecreasePendingEvents_)
{
}

TEST(QosManager, LogEvent_)
{
}

TEST(QosManager, GetEventLog_)
{
}

TEST(QosManager, GetPendingEvents_)
{
}

TEST(QosManager, GetVolBWLog_)
{
}

TEST(QosManager, LogVolBw_)
{
}

TEST(QosManager, SetEventPolicy_)
{
}

TEST(QosManager, ResetEventPolicy_)
{
}

TEST(QosManager, CopyEventPolicy_)
{
}

TEST(QosManager, AioSubmitAsyncIO_)
{
}

TEST(QosManager, GetEventId_)
{
}

TEST(QosManager, SetMaxVolWeightCli_)
{
}

TEST(QosManager, GetMaxVolumeWeight_)
{
}

TEST(QosManager, GetVolumeReactorMap_)
{
}

TEST(QosManager, GetVolumeTotalConnection_)
{
}

TEST(QosManager, VolumeQosPoller_)
{
}

TEST(QosManager, _Finalize_)
{
}

TEST(QosManager, _QosWorkerPoller_)
{
}

TEST(QosManager, _UpdatePolicyManagerActiveVolumeData_)
{
}

TEST(QosManager, _UpdateMaxVolumeWeightReactorVolume_)
{
}

TEST(QosManager, _HandleVolumeMinimumPolicy_)
{
}

TEST(QosManager, _GetVolumeFromActiveSubsystem_)
{
}

} // namespace pos

namespace pos
{
TEST(QosVolumeManager, QosVolumeManager_)
{
}

TEST(QosVolumeManager, VolumeCreated_)
{
}

TEST(QosVolumeManager, VolumeDeleted_)
{
}

TEST(QosVolumeManager, VolumeMounted_)
{
}

TEST(QosVolumeManager, VolumeUnmounted_)
{
}

TEST(QosVolumeManager, VolumeLoaded_)
{
}

TEST(QosVolumeManager, VolumeUpdated_)
{
}

TEST(QosVolumeManager, VolumeDetached_)
{
}

TEST(QosVolumeManager, UpdateSubsystemToVolumeMap_)
{
}

TEST(QosVolumeManager, GetVolumeFromActiveSubsystem_)
{
}

TEST(QosVolumeManager, AioSubmitAsyncIO_)
{
}

TEST(QosVolumeManager, SetVolumeWeight_)
{
}

TEST(QosVolumeManager, GetVolumeWeight_)
{
}

TEST(QosVolumeManager, DequeueVolumeParams_)
{
}

TEST(QosVolumeManager, VolumeQosPoller_)
{
}

TEST(QosVolumeManager, _EnqueueVolumeParams_)
{
}

TEST(QosVolumeManager, _ResetRateLimit_)
{
}

TEST(QosVolumeManager, _RateLimit_)
{
}

TEST(QosVolumeManager, _UpdateRateLimit_)
{
}

TEST(QosVolumeManager, _EnqueueVolumeUbio_)
{
}

TEST(QosVolumeManager, _UpdateVolumeMaxQos_)
{
}

TEST(QosVolumeManager, _DequeueVolumeUbio_)
{
}

} // namespace pos

namespace pos
{
TEST(QosEventManager, QosEventManager_)
{
}

TEST(QosEventManager, EventQosPoller_)
{
}

TEST(QosEventManager, SubmitAsyncIO_)
{
}

TEST(QosEventManager, DequeueEventParams_)
{
}

TEST(QosEventManager, GetEventWeight_)
{
}

TEST(QosEventManager, SetEventWeight_)
{
}

TEST(QosEventManager, GetEventWeightWRR_)
{
}

TEST(QosEventManager, SetEventWeightWRR_)
{
}

TEST(QosEventManager, _EventParamterInit_)
{
}

TEST(QosEventManager, _EnqueueEventParams_)
{
}

TEST(QosEventManager, _EnqueueEventUbio_)
{
}

TEST(QosEventManager, _DequeueEventUbio_)
{
}

TEST(QosEventManager, _ResetEventRateLimit_)
{
}

TEST(QosEventManager, _EventRateLimit_)
{
}

TEST(QosEventManager, _UpdateEventRateLimit_)
{
}

TEST(QosEventManager, _IdentifyEventType_)
{
}

} // namespace pos

namespace pos
{
TEST(QosSpdkManager, QosSpdkManager_)
{
}

TEST(QosSpdkManager, Initialize_)
{
}

TEST(QosSpdkManager, Finalize_)
{
}

TEST(QosSpdkManager, GetReactorData_)
{
}

TEST(QosSpdkManager, GetSpdkPoller_)
{
}

TEST(QosSpdkManager, UpdateReactorData_)
{
}

TEST(QosSpdkManager, UpdateSpdkPoller_)
{
}

TEST(QosSpdkManager, RegisterQosPoller_)
{
}

TEST(QosSpdkManager, PollerUnregister_)
{
}

TEST(QosSpdkManager, SpdkVolumeQosPoller_)
{
}

TEST(QosSpdkManager, GetReactorId_)
{
}

TEST(QosSpdkManager, SetReactorId_)
{
}

TEST(QosSpdkManager, _SetupQosReactorPoller_)
{
}

} // namespace pos
