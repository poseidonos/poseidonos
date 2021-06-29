#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_status.h"

namespace pos
{
class MockCopyInfo : public CopyInfo
{
public:
    using CopyInfo::CopyInfo;
};

class MockGcStatus : public GcStatus
{
public:
    using GcStatus::GcStatus;
};

} // namespace pos
