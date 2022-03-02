#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/include/allocator_const.h"

namespace pos
{
class MockCtxHeader : public CtxHeader
{
public:
    using CtxHeader::CtxHeader;
};

class MockAllocatorCtxHeader : public AllocatorCtxHeader
{
public:
    using AllocatorCtxHeader::AllocatorCtxHeader;
};

class MockSegmentCtxHeader : public SegmentCtxHeader
{
public:
    using SegmentCtxHeader::SegmentCtxHeader;
};

class MockRebuildCtxHeader : public RebuildCtxHeader
{
public:
    using RebuildCtxHeader::RebuildCtxHeader;
};

} // namespace pos
