#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_common.h"

namespace pos
{
class Mockbw_iops_parameter : public bw_iops_parameter
{
public:
    using bw_iops_parameter::bw_iops_parameter;
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

class Mockqos_rebuild_policy : public qos_rebuild_policy
{
public:
    using qos_rebuild_policy::qos_rebuild_policy;
};

class Mockqos_correction_type : public qos_correction_type
{
public:
    using qos_correction_type::qos_correction_type;
};

class MockQosReturnCode : public QosReturnCode
{
public:
    using QosReturnCode::QosReturnCode;
};

} // namespace pos
