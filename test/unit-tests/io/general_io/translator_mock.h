#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/translator.h"

namespace pos
{
class MockTranslator : public Translator
{
public:
    using Translator::Translator;
    MOCK_METHOD(PhysicalBlkAddr, GetPba, (uint32_t blockIndex), (override));
    MOCK_METHOD(PhysicalBlkAddr, GetPba, (), (override));
    MOCK_METHOD(list<PhysicalEntry>, GetPhysicalEntries, (void* mem, uint32_t blockCount), (override));
    MOCK_METHOD(StripeAddr, GetLsidEntry, (uint32_t blockIndex), (override));
    MOCK_METHOD(LsidRefResult, GetLsidRefResult, (uint32_t blockIndex), (override));
    MOCK_METHOD(bool, IsUnmapped, (), (override));
    MOCK_METHOD(bool, IsMapped, (), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVsa, (uint32_t blockIndex), (override));
};

} // namespace pos
