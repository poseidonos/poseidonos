#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metadata/segment_context_updater.h"

namespace pos
{
class MockSegmentContextUpdater : public SegmentContextUpdater
{
public:
    using SegmentContextUpdater::SegmentContextUpdater;
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool isForced), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
};

} // namespace pos
