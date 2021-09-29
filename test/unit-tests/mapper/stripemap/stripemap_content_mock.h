#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/stripemap/stripemap_content.h"

namespace pos
{
class MockStripeMapContent : public StripeMapContent
{
public:
    using StripeMapContent::StripeMapContent;
    MOCK_METHOD(int, InMemoryInit, (uint64_t entrySize, uint64_t mpageSize), (override));
    MOCK_METHOD(MpageList, GetDirtyPages, (uint64_t start, uint64_t numEntries), (override));
    MOCK_METHOD(StripeAddr, GetEntry, (StripeId vsid), (override));
    MOCK_METHOD(int, SetEntry, (StripeId vsid, StripeAddr entry), (override));
};

} // namespace pos
