#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/fake/array_partation.h"

namespace pos
{
class MockDeviceLba : public DeviceLba
{
public:
    using DeviceLba::DeviceLba;
};

class MockLogicalBlkAddr : public LogicalBlkAddr
{
public:
    using LogicalBlkAddr::LogicalBlkAddr;
};

class MockSizeInfo : public SizeInfo
{
public:
    using SizeInfo::SizeInfo;
};

class MockArrayPartation : public ArrayPartation
{
public:
    using ArrayPartation::ArrayPartation;
};

} // namespace pos
