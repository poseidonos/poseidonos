#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/merger.h"

namespace pos
{
class MockMerger : public Merger
{
public:
    using Merger::Merger;
    MOCK_METHOD(void, Add, (PhysicalBlkAddr & pba, VirtualBlkAddr& vsa, StripeAddr& lsidEntry, uint32_t targetSize), (override));
    MOCK_METHOD(void, Cut, (), (override));
    MOCK_METHOD(uint32_t, GetSplitCount, (), (override));
    MOCK_METHOD(VolumeIoSmartPtr, GetSplit, (uint32_t index), (override));
};

} // namespace pos
