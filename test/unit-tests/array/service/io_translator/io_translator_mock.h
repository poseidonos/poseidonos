#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_translator/io_translator.h"

namespace pos
{
class MockIOTranslator : public IOTranslator
{
public:
    using IOTranslator::IOTranslator;
    MOCK_METHOD(int, Translate, (string array, PartitionType part, PhysicalBlkAddr& dst, const LogicalBlkAddr& src), (override));
    MOCK_METHOD(int, Convert, (string array, PartitionType part, list<PhysicalWriteEntry>& dst, const LogicalWriteEntry& src), (override));
};

} // namespace pos
