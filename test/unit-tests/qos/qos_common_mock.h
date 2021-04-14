#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_common.h"

namespace pos
{
class MockQosReturnCode : public QosReturnCode
{
public:
    using QosReturnCode::QosReturnCode;
};

class Mockvolume_qos_params : public volume_qos_params
{
public:
    using volume_qos_params::volume_qos_params;
};

class Mockevent_qos_params : public event_qos_params
{
public:
    using event_qos_params::event_qos_params;
};

class Mockpoller_structure : public poller_structure
{
public:
    using poller_structure::poller_structure;
};

class Mockqos_vol_policy : public qos_vol_policy
{
public:
    using qos_vol_policy::qos_vol_policy;
};

class Mockqos_state_ctx : public qos_state_ctx
{
public:
    using qos_state_ctx::qos_state_ctx;
};

class MockoldVolState : public oldVolState
{
public:
    using oldVolState::oldVolState;
};

class MockvolState : public volState
{
public:
    using volState::volState;
};

class MockflushEventData : public flushEventData
{
public:
    using flushEventData::flushEventData;
};

class MockgcEventData : public gcEventData
{
public:
    using gcEventData::gcEventData;
};

class MockrebuildEventData : public rebuildEventData
{
public:
    using rebuildEventData::rebuildEventData;
};

class MockmetaEventData : public metaEventData
{
public:
    using metaEventData::metaEventData;
};

class MockoldEventState : public oldEventState
{
public:
    using oldEventState::oldEventState;
};

class MockeventState : public eventState
{
public:
    using eventState::eventState;
};

} // namespace pos
