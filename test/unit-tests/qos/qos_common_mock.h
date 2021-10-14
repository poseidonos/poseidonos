#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/qos_common.h"

namespace pos
{
class MockbwIopsParameter : public bw_iops_parameter
{
public:
    using bw_iops_parameter::bw_iops_parameter;
};

class MockpollerStructure : public poller_structure
{
public:
    using poller_structure::poller_structure;
};

class MockqosVolPolicy : public qos_vol_policy
{
public:
    using qos_vol_policy::qos_vol_policy;
};

class MockqosRebuildPolicy : public qos_rebuild_policy
{
public:
    using qos_rebuild_policy::qos_rebuild_policy;
};

class MockqosCorrectionType : public qos_correction_type
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
